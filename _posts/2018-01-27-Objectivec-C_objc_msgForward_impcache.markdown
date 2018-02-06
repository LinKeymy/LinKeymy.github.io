---
layout: post
title: Objective-C:_objc_msgForward_impcache
subtitle: 
author: JackLin
date: 2018-01-27 14:49:22 +0800
---

如果你对lookUpImpOrForward有所了解，会发现在经过类、父类、方法决议都没有查找到`sel`对应的实现后，就会执行:

```oc
imp = (IMP)_objc_msgForward_impcache;
cache_fill(cls, sel, imp, inst);
```

并将imp(_objc_msgForward_impcache)返回. _ objc_msgForward_impcache中具体会执行上面呢？在objc-private.h里面查到:

```oc
/* message dispatcher */
extern IMP _class_lookupMethodAndLoadCache3(id, SEL, Class);
#if !OBJC_OLD_DISPATCH_PROTOTYPES
extern void _objc_msgForward_impcache(void);
#else
extern id _objc_msgForward_impcache(id, SEL, ...);
#endif
```

很不幸， _objc_msgForward_impcache是一个私有的`message dispatcher`。   函数类型为(void (*)void).   看来 _objc_msgForward_impcache仅提供给派发器（dispatcher）用于处理消息转发的函数。 _objc_msgForward_impcache目测是没有开源，这里无法查看它的具体实现。但是由上层的一些API还可以看看大概的处理过程的。在<NSObject>协议中定义了如下三个方法进行消息转发，同时NSObject类也提供类默认的实现，但都是抛出异常信息。

```
- (id)forwardingTargetForSelector:(SEL)aSelector OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
- (void)forwardInvocation:(NSInvocation *)anInvocation OBJC_SWIFT_UNAVAILABLE("");
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector OBJC_SWIFT_UNAVAILABLE("");
```



* `forwardingTargetForSelector: `返回一个target 用于接收被当前类转发的消息，无法改变sel，但是target必须改变，不能是self。
* `methodSignatureForSelector：`根据需要返回一个方法的签名(要和Invocation中的selector匹配)


* `forwardInvocation:` 对原来sel进行签名打包，可以改变sel也可以保留，然后invocate一个target。这里target可以是self保持不变，也可以是其他对象。

###  forwardingTargetForSelector： 

> 
>
> * **想直接转发一个对象，实现NSObject协议中的`forwardingTargetForSelector：`**
>
>

>Returns the object to which unrecognized messages should first be directed.
>
>If an object implements (or inherits) this method, and returns a non-`nil` (and non-`self`) result, that returned object is used as the new receiver object and the message dispatch resumes to that new object. (Obviously if you return `self` from this method, the code would just fall into an infinite loop.)
>
>If you implement this method in a non-root class, if your class has nothing to return for the given selector then you should return the result of invoking super’s implementation. 

> This method gives an object a chance to redirect an unknown message sent to it before the much more expensive [forwardInvocation:](apple-reference-documentation://hcR2PGYTDL) machinery takes over. This is useful when you simply want to redirect messages to another object and can be an order of magnitude faster than regular forwarding. It is not useful where the goal of the forwarding is to capture the NSInvocation, or manipulate the arguments or return value during the forwarding.

从apple给出的description中可以看出`forwardingTargetForSelector:`的使用要注意以下几点;

* **不能返回self或者当前类的对象，否则会进入死循环**
* **如果super上实现（non-root class）返回super的实现就好**
* **不能返回nil(因为NSObject中的默认实现就是返回nil)，如果返回nil会进入 forwardInvocation**

>NSObject中的默认实现就是返回nil的哦，请看截图

![有帮助的截图]({{ site.url }}/assets/postsImages/nsobject_forwarding.jpg)

_objc_msgForward_impcache如何调用forwardingTargetForSelector的呢？新添加一个ForwardClass实现了 `forwardingTargetForSelector: ` 用于接收处理Farter转发过来的消息，其具体代码如下:

```oc


@implementation Farter

- (id)forwardingTargetForSelector:(SEL)aSelector {
    if (NSSelectorFromString(@"breakWind") == aSelector) {
        return [ForwardClass new];
    }
    return  [super forwardingTargetForSelector:aSelector];
}

@end

#import "ForwardClass.h"

@implementation ForwardClass

- (void)breakWind {
    NSLog(@"forward to break wind");
}

@end
#import <Foundation/Foundation.h>
#import "Farter.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [Farter new];
        [f breakWind];
        [f breakWind];
        [f breakWind];
    return 0;
    }
}

```

在main.m中尝试发送消息给Farter类型的对像，并且`forwardingTargetForSelector:`中打个断点，尝试查看调用栈的信息：

![有帮助的截图]({{ site.url }}/assets/postsImages/forwarding_taget_stop01.jpg)

可以看到调用栈:

0  `[Farter forwardingTargetForSelector:]`

1 `__forwarding__`

2 `__forwarding_prep_0___`

进入`__forwarding__`尝试查看一下有用的信息：

![有帮助的截图]({{ site.url }}/assets/postsImages/forwarding_taget_stop02.jpg)

从右边的汇编注释可以看出，调用了`object_getClass`、`class_getName`，有拿`"forwardingTargetForSelector:"`字符串，并且调用了`class_respondsToSelector`.在`class_respondsToSelector`中打一来个条件断点(一个是开始，一个是结束，这样方便看)，并且p一些信息



![有帮助的截图]({{ site.url }}/assets/postsImages/class_respondsToSelector_start.png)

![有帮助的截图]({{ site.url }}/assets/postsImages/class_respondsToSelector_end.png)

为了观察消息过程的更多细节，在`lookUpImpOrForward`中也添加一个断点:

![有帮助的截图]({{ site.url }}/assets/postsImages/lookUpImpOrForward_p_detail.png)



一切准备就绪，run起来:

![有帮助的截图]({{ site.url }}/assets/postsImages/forwardingTargetForSelector_printf.jpg)

> * 三次的`breakWind`消息都被转发
> * 在转发前调用`forwardingTargetForSelector:`先对其进行检查class_respondsToSelector(cls,@selector(forwardingTargetForSelector))
> * 第一次转发objc_msgForward_impcache没有被缓存到类结构的cachet_t里面

上面的第二点对于`NSObject`或者其之类来说class_respondsToSelector检查结果总是`YES`.同时`NSObject`有了默认的实现返回nil，但是为了兼顾其他可能非NSObject类和安全考虑还是值得进行这一步检查。

### forwardInvocation

如果在对forwardingTargetForSelector:返回的结果为nil,那么就会进入`forwardInvocation`阶段，这里要自己实现如下两个方法:

>- **(void)forwardInvocation:(NSInvocation *)anInvocation**
>- **(NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector**



`methodSignatureForSelector`方法是返回一个方法签名，在NSObject中的默认实现是调用_objc_fatal抛出异常,从注释`Replaced by CF`看出已经被CoreFoundation框架的实现替代了。

```oc
// Replaced by CF (returns an NSMethodSignature)
+ (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    _objc_fatal("+[NSObject methodSignatureForSelector:] "
                "not available without CoreFoundation");
}

// Replaced by CF (returns an NSMethodSignature)
- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    _objc_fatal("-[NSObject methodSignatureForSelector:] "
                "not available without CoreFoundation");
}

```

所以要使用`forwardInvocation`:必须提供`methodSignatureForSelector:`实现，而且不能返回nil，对于方法签名可以和传入的aSelector不同，具体的签名是要`anInvocation.selector`相匹配。

`forwardInvocation：`方法也是类似，在NSObject中的默认实现如下：

```oc
+ (void)forwardInvocation:(NSInvocation *)invocation {
    [self doesNotRecognizeSelector:(invocation ? [invocation selector] : 0)];
}

- (void)forwardInvocation:(NSInvocation *)invocation {
    [self doesNotRecognizeSelector:(invocation ? [invocation selector] : 0)];
}
```

关于所`forwardInvocation:`和`methodSignatureForSelector:`更多的细节可以查看apple的文档，接下来看看整个转发过程，为此将Farter的`implementation`更改：

```oc

#import "Farter.h"

@implementation Farter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    if ((NSSelectorFromString(@"breakWind")) == aSelector) {
        return  [NSMethodSignature signatureWithObjCTypes:"v@:@"];
    }
    return [super methodSignatureForSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    ForwardClass *forword = [ForwardClass new];
    anInvocation.target = forword;
    anInvocation.selector = sel_registerName("getMessageFromFarter:");
    NSString *way = @"help me to break wind";
    [anInvocation setArgument:&way atIndex:2];
    [anInvocation invoke];
}

@end
```

查看第一次转发和第二次的部分转发的打印结果:

![有帮助的截图]({{ site.url }}/assets/postsImages/forwardInvocation01.jpg)

> * 两次都查询了breakwind，看来第一次转发缓存起了的_objc_msgForward_impcache在后面缓存其他方法缓存空间扩容的时候被刷掉了。

因为子NSObject都有了实现，所以对三个方法进行class_respondsToSelector(cls,sel)检查总是返回YES，正常流程无法得知_objc_msgForward_impcache对NO如何处理，为了验证NO的情况，在class_respondsToSelector修改返回值，遇到Farter的cls总是返回NO:

![有帮助的截图]({{ site.url }}/assets/postsImages/class_respondsToSelector_bad.png)

查看非正常情况下的打印结果:

![有帮助的截图]({{ site.url }}/assets/postsImages/class_respondsToSelector_bad_po.jpg)

结合前面的看来`_objc_msgForward_impcache `函数处理逻辑大概如下：

* 检查是否实现了`forwardingTargetForSelector:`，若实现了就调用，将消息转发给返回的对象

* 若没有没有实现`forwardingTargetForSelector:`或者`forwardingTargetForSelector:`返回nil就检查`methodSignatureForSelector:`是否实现

* 若`methodSignatureForSelector:`有实现就调用`methodSignatureForSelector:`获得方法的签名(返回nil也会抛出异常)。没有实现就抛出异常，类似如下：

  > ***\** NSForwarding: warning: object 0x100b7ffd0 of class 'Farter' does not implement methodSignatureForSelector: -- trouble ahead**

* 若从`methodSignatureForSelector:`中获得有效的`NSInvocation`对象，调用`forwardInvocation:`并将`NSInvocation`对象作为参数传入。到这样_objc_msgForward_impcache就完成了整个转发的处理

  ​

总结`_objc_msgForward_impcache`的整个处理逻辑，扒出自己猜测_objc_msgForward_impcache的实现:

```oc
void alin_objc_msgForward_impcache() {
    id obj;
    SEL aselector = sel_registerName("aselector");
    Class cls = objc_getClass("name");
    BOOL isForwarding = class_respondsToselector(cls,@selector(forwardingTargetForSelector:));
    id forwardObj;
    if (isForwarding) {
        // 如果当前类或者其父类(非NSObject)没有实现，执行NSObject的实现返回nil
        forwardObj =  [obj forwardingTargetForSelector:aselector];
        if (forwardObj) {
             objc_msgSend(forwardObj, aselector);
        }
    } else {
        BOOL isSig = class_respondsToselector(cls,@selector(methodSignatureForSelector:));
        if (isSig) { // 如果有实现methodSignatureForSelector:
            // 如果当前类或者其父类(非NSObject)没有实现，调用NSObject的导致抛出异常
            NSMethodSignature *methodSig =  [obj methodSignatureForSelector:aselector];
            if (methodSig) { // 如过methodSignatureForSelector返回non-nil
                BOOL isInvocation = class_respondsToselector(cls,@selector(forwardInvocation:));
                if (isInvocation) { // 如果有实现forwardInvocation:
                    NSInvocation *aInvocation = [NSInvocation invocationWithMethodSignature:methodSig];
                    // 如果当前类或者其父类(非NSObject)没有实现，调用NSObject的导致抛出异常
                    [obj forwardInvocation:aInvocation];
                } else { // 如果没有实现forwardInvocation:
                    _objc_fatal("*** NSForwarding: warning: object 0x100a8ff30 of class 'Farter' does not implement forwardInvocation: -- dropping message");
                }
            } else { // 如果methodSignatureForSelector返回nil，抛出异常
                [NSObject doesNotRecognizeSelector:aselector];
            }
        } else { // 如果没有实现methodSignatureForSelector:
            _objc_fatal("** NSForwarding: warning: object 0x100b7ffd0 of class 'Farter' does not implement methodSignatureForSelector: -- trouble ahead");
        }
    }
}
```

通过正常和非常情况下的调试，_objc_msgForward_impcache的逻辑应该就是上面的处理逻辑了。

#### the end ，code night









