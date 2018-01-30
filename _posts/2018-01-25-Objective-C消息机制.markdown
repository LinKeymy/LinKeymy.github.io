---
layout: post
title: Objective-C:消息机制
subtitle: 
author: JackLin
date: 2018-01-25 15:39:56 +0800
---






>因为 ObjC 的 runtime 只能在 Mac OS 下才能编译，所以文章中的代码都是在 Mac OS，也就是 x86_64 架构下运行的，对于在 arm64 中运行的代码会特别说明。


* 你可以在[objc-runtime](https://github.com/RetVal/objc-runtime)clone 整个仓库来进行调试objc-runtime工程进行调试。
* 关于LLDB的一些使用感兴趣的可以查阅这里[The LLDB Debugger](http://lldb.llvm.org/tutorial.html)还有这里[iOS开发调试 - LLDB使用概览](https://www.jianshu.com/p/67f08a4d8cf2)


### 前言

曾几何时，你是否和笔者一样好奇[receiver message]到底发生了什么？对象是如何找到对应类的方法，或者是从父类中继承的方法，SEL具体又是什么？......这一切又是如何结合在一起发生作用的呢？如果是，或许这篇文章会解决你部分的疑惑，帮助你看了解消息发送的细节和处理过程。


### [receiver message]

创建一个Farter类，为其添加`helloworld`,并在main.n中使用，如下

```oc

//Farter.m
#import "Farter.h"

@implementation Farter

- (void)helloworld {
    NSLog(@"helloworld");
}

@end

//main.m
#import <Foundation/Foundation.h>
#import "Farter.h"
#import <objc/message.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *object = [Farter new];
        [object helloworld];
    return 0;
    }
}

```
上面main.m在Xcode内置编译器clang处理后的中间代码是怎么样的如何呢？我们使用clang -rewrite-objc 把oc代码转写成c/c++代码,输出的对应代码如下:

```oc
int main(int argc, const char * argv[]) {
    /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 
        Farter *object = ((Farter *(*)(id, SEL))(void *)objc_msgSend)((id)objc_getClass("Farter"), sel_registerName("new"));
        ((void (*)(id, SEL))(void *)objc_msgSend)((id)object, sel_registerName("helloworld"));
    return 0;
    }
}

```

> 重点关注 ((void (*)(id, SEL))(void *)objc_msgSend)((id)object, sel_registerName("helloworld"));


简化后的objc_msgSend(object, sel_registerName("helloworld")),这里sel_registerName函数作用和 @selector(helloworld)一样是生成的选择子 SEL 不同的是sel_registerName在动态运行时生成的选择子，而@selector()是编译期间就声明的选择子。可以说上面等价于:
> objc_msgSend(f, @selector(helloworld))

可以看出Objective-C 中，所有的消息传递中的“消息“都会被转换成一个 selector 作为 objc_msgSend 函数的参数：

[object helloworld]    ==>>  objc_msgSend(object, @selector(helloworld))



### @selector() 和 SEL
使用 @selector(helloworld) 生成的选择子 SEL，那么SEL是什么？
> SEL你可以看作是一个函数名映射的一个字符串，就是一个字符串
> 那问题又来了

* 使用 `@selector(helloworld)` 生成的选择子，是否会因为类的不同而不同？
* 使用 `@selector(helloworld)` 生成的选择子，是否会因(+)类方法和(-)对象方法的不同而不同？


从表达式`@selector(helloworld)`和函数`sel_registerName("helloworld")`,传入的信息仅仅是`helloworld`这样的一个字符串，没有关联的类或者对象，也没有其他方法类型相关的信息。初步可以推出选择子的生成只和方法的名字有关(这里名字包括带有参数是的`:`)如：
`helloworld`和`helloworld:`以及`helloworld::`不同。

在mian函数[object hello]位置打一个断点，然后在 lldb 中输入：

![有帮助的截图]({{ site.url }}/assets/postsImages/message_selector_l.png)


@selector(helloworld)和@selector(build_get_selector)是在编译期间就声明的选择子，而@selector(undefined_selector)在编译期间并不存在，选择子由于是在运行时生成的，所以内存地址明显比 build_get_selector和helloworld大很多,而且执行sel_registerName("undefined_selector")多少次都是一样的地址。

>从上面打印的结果这，可以推断出选择子有以下的特性：

* Objective-C 为我们维护了一个选择子表
* 在使用 @selector()、sel_registerName() 时会从这个选择子表中根据选择子的名字查找对应的SEL。如果没有找到，则会生成一个 SEL 并添加到表中
* 在编译期间会扫描全部的头文件和实现文件将其中的方法以及使用 @selector()生成的选择子加入到选择子表中


### objc_msgSend 调用栈

因为 objc_msgSend 是一个私有方法，我们没有办法进入它的实现，但是，我们却可以在 objc_msgSend 的调用栈中“截下”这个函数调用的过程。既然要执行对应的方法，肯定要寻找选择子对应的实现。在 objc-runtime-new.mm 文件中有一个函数 lookUpImpOrForward，这个函数的作用就是查找方法的实现.在 [object helloworld] 这里增加一个断点，当程序运行到这一行时，再向 lookUpImpOrForward 函数的第一行添加断点，确保是捕获 @selector(hello) 的调用栈，而不是调用其它选择子的调用栈。如下图

![有帮助的截图]({{ site.url }}/assets/postsImages/lookUpImpOrForward01.jpeg)


随后继续执行代码，会在lookUpImpOrForward中的断点停下，使用ddlb打印cls对应的类为Farter，传入的选择子sel为 "helloworld"。可以确信这就是当调用 helloworld 方法时执行的函数:


![有帮助的截图]({{ site.url }}/assets/postsImages/lookUpImpOrForward02.jpeg)


在Xcode左边查看调用栈:

>
>0  lookUpImpOrForward  
>1  _class_lookupMethodAndLoadCache3    
>2  objc_msgSend  
>3  main  
>4  start



调用栈在这里告诉我们： lookUpImpOrForward 并不是 objc_msgSend 直接调用的，而是通过 _class_lookupMethodAndLoadCache3 方法，在objc-runtime-new.mm 文件中查找到_class_lookupMethodAndLoadCache3方法:


```oc
/***********************************************************************
* _class_lookupMethodAndLoadCache.
* Method lookup for dispatchers ONLY. OTHER CODE SHOULD USE lookUpImp().
* This lookup avoids optimistic cache scan because the dispatcher 
* already tried that.
**********************************************************************/
IMP _class_lookupMethodAndLoadCache3(id obj, SEL sel, Class cls)
{
    return lookUpImpOrForward(cls, sel, obj, 
                              YES/*initialize*/, NO/*cache*/, YES/*resolver*/);
}

```
注释说得很清楚，`_class_lookupMethodAndLoadCache`方法仅仅提供给dispatcher使用，其它的代码都应该使用 lookUpImpOrNil()（不会进行方法转发），传入 cache = NO 避免在没有加锁的时候对缓存进行查找，因为派发器已经做过这件事情了。


### lookUpImpOrForward


下面是lookUpImpOrForward方法的实现，已经整理并修改了注释

```oc
IMP lookUpImpOrForward(Class cls, SEL sel, id inst, 
                       bool initialize, bool cache, bool resolver)
{
    IMP imp = nil;
    bool triedResolver = NO;
	//在没有加锁的时候对缓存进行查找，提高缓存使用的性能
    runtimeLock.assertUnlocked();
    if (cache) {
        imp = cache_getImp(cls, sel);
        if (imp) return imp;
    }


    runtimeLock.read();

    if (!cls->isRealized()) {
        runtimeLock.unlockRead();
        runtimeLock.write();

        realizeClass(cls);

        runtimeLock.unlockWrite();
        runtimeLock.read();
    }

    if (initialize  &&  !cls->isInitialized()) {
        runtimeLock.unlockRead();
        _class_initialize (_class_getNonMetaClass(cls, inst));
        runtimeLock.read();
    }
    
 retry:    
    runtimeLock.assertReading();

	// 先从当前类的缓存里面查找
    imp = cache_getImp(cls, sel);
    if (imp) goto done;

    // 尝试在当前类的方法列表里面查找
    {
        Method meth = getMethodNoSuper_nolock(cls, sel);
        if (meth) {
            log_and_fill_cache(cls, meth->imp, sel, inst, cls);
            imp = meth->imp;
            goto done;
        }
    }

    // 尝试在在父类的方法列表里面查找
    {
        unsigned attempts = unreasonableClassCount();
        for (Class curClass = cls->superclass;
             curClass != nil;
             curClass = curClass->superclass)
        {
            // 如果是根类，就_objc_fatal
            if (--attempts == 0) {
                _objc_fatal("Memory corruption in class list.");
            }
            
            // Superclass的缓存查找.
            imp = cache_getImp(curClass, sel);
            if (imp) {
                if (imp != (IMP)_objc_msgForward_impcache) {
                    // Found the method in a superclass. Cache it in this class.
                    log_and_fill_cache(cls, imp, sel, inst, curClass);
                    goto done;
                }
                else {
                    // Found a forward:: entry in a superclass.
                    // Stop searching, but don't cache yet; call method 
                    // resolver for this class first.
                    break;
                }
            }
            // Superclass 方法列表查找
            Method meth = getMethodNoSuper_nolock(curClass, sel);
            if (meth) {
                log_and_fill_cache(cls, meth->imp, sel, inst, curClass);
                imp = meth->imp;
                goto done;
            }
        }
    }

    if (resolver  &&  !triedResolver) {
        runtimeLock.unlockRead();
        _class_resolveMethod(cls, sel, inst); //  尝试方法决议resolver
        runtimeLock.read();
        triedResolver = YES;
        goto retry;  // 如果resolver帮忙，返回查找
    }

    // implementation没有找到，resolver也不帮忙，那么久使用forwarding
    imp = (IMP)_objc_msgForward_impcache;
    cache_fill(cls, sel, imp, inst);
 done:
    runtimeLock.unlockRead();
    return imp;
}

```

* cache = 0下面的if会直接跳过

```oc
	//在没有加锁的时候对缓存进行查找，提高缓存使用的性能
    runtimeLock.assertUnlocked();
    if (cache) {
        imp = cache_getImp(cls, sel);
        if (imp) return imp;
    }
```
* 处理类的实现和初始化的工作

```oc
    runtimeLock.read();

    if (!cls->isRealized()) { // 类实现
        runtimeLock.unlockRead();
        runtimeLock.write();

        realizeClass(cls);

        runtimeLock.unlockWrite();
        runtimeLock.read();
    }

    if (initialize  &&  !cls->isInitialized()) { //类的初始化
        runtimeLock.unlockRead();
        _class_initialize (_class_getNonMetaClass(cls, inst));
        runtimeLock.read();
    }
```

* 先从当前类的缓存列表里面查找，如果找到，完成查找。

```oc
    imp = cache_getImp(cls, sel);
    if (imp) goto done;
```
由于`helloworld`是第一次调用查找，应该缓存没有，cache_getImp(cls, sel)执行后打印imp的值为空也证实了确实没有。

![有帮助的截图]({{ site.url }}/assets/postsImages/cache_getImp_uncached.png)


* 若缓存列表中没有找到，尝试在当前类的方法列表里面查找

 继续往下执行，调用getMethodNoSuper_nolock(cls, sel)在当前类中查找，其实现如下:

 ```oc

static method_t *getMethodNoSuper_nolock(Class cls, SEL sel)  
{
    runtimeLock.assertLocked();
    assert(cls->isRealized());
    for (auto mlists = cls->data()->methods.beginLists(), 
              end = cls->data()->methods.endLists(); 
         mlists != end;
         ++mlists)
    {
        method_t *m = search_method_list(*mlists, sel);
        if (m) return m;
    }
    return nil;
}
 ```
 从search_method_list()的方法名称和传入的参数可以猜到就是根据对应的方法签名到方法列表查找method_t结构体：
 ```oc
struct method_t {
    SEL name;
    const char *types;
    IMP imp;
};
 ```
search_method_list(),就是根据 method_t中的name和传入的sel匹配找到method_t，而且需要的imp就在其中。

![有帮助的截图]({{ site.url }}/assets/postsImages/getMethodNoSuper_nolock01.jpeg)

打印`meth`发现找到了对应的method_t，打印 `meth->name`发现就要要找的`helloworld`.既然已经找到，那么先将其缓存起来;

```oc
if (meth) {
            log_and_fill_cache(cls, meth->imp, sel, inst, cls);
            imp = meth->imp;
            goto done;
        }
    }

```
加入缓存后，下次就可以直接从缓存中命中方法了，未来确定，在控制台中使用lldb验证一下:

![有帮助的截图]({{ site.url }}/assets/postsImages/log_and_fill_cache_end.png)

现在使用`cache_getImp(cls, sel)`可以直接`get`到imp，而且发现和刚刚获取到的imp的地址是一样的.那么以后再次发送`helloworld`消息，直接在缓存命中，跳出查找，然后执行。

* 如果当前类没有找到，在父类中寻找实现

这一步与上面相似现，只是多了一个循环用来判断根类，在没有找到前，一直循环:
> 1.缓存查找
> 2.方法列表中查找
> 3.在父类中查找

需要注意的是，在父类网上的继承链上查找过程的`_objc_msgForward_impcache`还是由最初消息接受者的类处理,既是转发任务还是当前类负责。

```oc
{
        unsigned attempts = unreasonableClassCount();
        for (Class curClass = cls->superclass;
             curClass != nil;
             curClass = curClass->superclass)
        {
            // Halt if there is a cycle in the superclass chain.
            if (--attempts == 0) {
                _objc_fatal("Memory corruption in class list.");
            }
            
            // Superclass cache.
            imp = cache_getImp(curClass, sel);
            if (imp) {
                if (imp != (IMP)_objc_msgForward_impcache) {
                    log_and_fill_cache(cls, imp, sel, inst, curClass);
                    goto done;
                }
                else {
                    break;
                }
            }
            
            // Superclass method list.
            Method meth = getMethodNoSuper_nolock(curClass, sel);
            if (meth) {
                log_and_fill_cache(cls, meth->imp, sel, inst, curClass);
                imp = meth->imp;
                goto done;
            }
        }
    }
```

选择子在当前类和父类往上继承链一直到根类都没有找到，就会进入了方法决议（method resolve）的过程

* 方法决议 (method resolve)

> No implementation found. Try method resolver once,

```oc
    // No implementation found. Try method resolver once.
    if (resolver  &&  !triedResolver) {
        runtimeLock.unlockRead();
        _class_resolveMethod(cls, sel, inst);
        runtimeLock.read();
        // Don't cache the result; we don't hold the lock so it may have 
        // changed already. Re-do the search from scratch instead.
        triedResolver = YES;
        goto retry;
    }
    
```

* _class_resolveMethod里面判断如果是isMetaClass那么调用: _class_resolveClassMethod,否则就调用：_class_resolveInstanceMethod


```oc

void _class_resolveMethod(Class cls, SEL sel, id inst)
{
    if (! cls->isMetaClass()) {
        // try [cls resolveInstanceMethod:sel]
        _class_resolveInstanceMethod(cls, sel, inst);
    } 
    else {
        // try [nonMetaClass resolveClassMethod:sel]
        // and [cls resolveInstanceMethod:sel]
        _class_resolveClassMethod(cls, sel, inst);
        if (!lookUpImpOrNil(cls, sel, inst, 
                            NO/*initialize*/, YES/*cache*/, NO/*resolver*/)) 
        {
            _class_resolveInstanceMethod(cls, sel, inst);
        }
    }
}
```

* _class_resolveClassMethod，查看

```oc
static void _class_resolveClassMethod(Class cls, SEL sel, id inst)
{
    assert(cls->isMetaClass()); 
    if (! lookUpImpOrNil(cls, SEL_resolveClassMethod, inst, 
                         NO/*initialize*/, YES/*cache*/, NO/*resolver*/)) 
    {
        // Resolver 没有实现直接return
        return;
    }
    BOOL (*msg)(Class, SEL, SEL) = (__typeof__(msg))objc_msgSend;
    bool resolved = msg(_class_getNonMetaClass(cls, inst), 
                        SEL_resolveClassMethod, sel);

    // 缓存起来，这样下次Resolver就不会再被开启
    // +resolveClassMethod adds to self->ISA() a.k.a. cls
    IMP imp = lookUpImpOrNil(cls, sel, inst, 
                             NO/*initialize*/, YES/*cache*/, NO/*resolver*/);
}
```

* _class_resolveInstanceMethod 

```oc
static void _class_resolveInstanceMethod(Class cls, SEL sel, id inst)
{
    if (! lookUpImpOrNil(cls->ISA(), SEL_resolveInstanceMethod, cls, 
                         NO/*initialize*/, YES/*cache*/, NO/*resolver*/)) 
    {
        // Resolver 没有实现，直接退出
        return;
    }
    BOOL (*msg)(Class, SEL, SEL) = (__typeof__(msg))objc_msgSend;
    bool resolved = msg(cls, SEL_resolveInstanceMethod, sel);

    IMP imp = lookUpImpOrNil(cls, sel, inst, 
                             NO/*initialize*/, YES/*cache*/, NO/*resolver*/);
}
```

在resolveInstanceMethod里面，一般会给sel添加实现，这样在发送msgresolveInstanceMethod对应的消息SEL_resolveInstanceMethod后，（msg(cls, SEL_resolveInstanceMethod, sel);）在场lookUpImpOrNil查找就应该可以找到了。如果没有，也不会再次进入方法决议了，因为入口的triedResolver设置为yes。如果经过方法决议还是没有找到imp，那么就会进入消息转发阶段。

* 消息转发
  在缓存、当前类、父类以及 resolveInstanceMethod: 都没有解决实现查找的问题时，Objective-C 还为我们提供了最后一次翻身的机会，进行方法转发：


```oc
imp = (IMP)_objc_msgForward_impcache;
cache_fill(cls, sel, imp, inst);
```
返回实现 _objc_msgForward_impcache，然后加入缓存。


这篇文章与其说是讲 ObjC 中的消息发送的过程，不如说是讲方法的实现是如何查找的,最后附图一张:

![有帮助的截图]({{ site.url }}/assets/postsImages/message_logic.png)

#### 祝大家生活愉快，晚安
