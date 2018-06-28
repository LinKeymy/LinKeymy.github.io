---
layout: post
title: Objectiv-C:KVO
subtitle: 
author: JackLin
date: 2018-03-23 10:56:54 +0800
---

### 前言

KVO 作为 iOS 中一种强大并且有效的机制，为 iOS 开发者们提供了很多的便利；我们可以使用 KVO 来检测对象属性的变化、快速做出响应，这能够为我们在开发强交互、响应式应用以及实现视图和模型的双向绑定时提供大量的帮助。在探讨KVO实现之前，我们先来回忆一下，在通常情况下，我们是如何使用 KVO 进行键值观测的。

### 如何使用KVO

1. 在观察者中重写下面这个方法：

```objective-c
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context;
```

2. 调用addOberver添加观察者：

```objective-c
[obj addObserver:observer forKeyPath:aKeyPath options:options context:nil];
```

3. 在适当的时机移除观察者

```objective-c
[obj removeObserver:observer forKeyPath:aKeyPath];
```

### addObserver后发生了什么？

创建两个类：`Observer"` 和`Boy`他们的具体代码如下：

```objective-c
#import "Observer.h"

@implementation Observer

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context {
    
    NSLog(@"\nobserve---- keyPath: %@, object: %@, change: %@",keyPath,object,change);
    
}
@end
    

@interface Boy : NSObject

@property (nonatomic,copy) NSString *girlFriend;

@end

```

分别实例化`Boy`和`Observer`两个对象`alin`、`observer`，使用`observer`来监听`alin`的`girlFriend`属性的变化。

```objective-c
#import <Foundation/Foundation.h>
#import "Boy.h"
#import "Observer.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        Boy *alin = [[Boy alloc] init];
        Boy *jack = [[Boy alloc] init];
        alin.girlFriend = @"Objective-C";
        jack.girlFriend = @"Rose";
        
        Observer *observer = [[Observer alloc] init];
        
        NSKeyValueObservingOptions options =
        NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
        
        [alin addObserver:observer forKeyPath:@"girlFriend" options:options context:nil];
        alin.girlFriend = @"Swift";
        [alin removeObserver:observer forKeyPath:@"girlFriend"];
    }
    return 0;
}
```

如下图，在main.m中添加两个断点，在断点处打印出`alin`和`jack`的isa指针，查看实例对象的`isa`指向的`Class`对象。

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_NSKVONotifying_Boy.png)

从结果可以看出`alin`对象执行方法`[alin addObserver:observer forKeyPath:@"girlFriend" options:options context:nil];`后`isa`的指向发生了改变。前后的关联逻辑：

执行addObserver前

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_noneObserver.png)

执行addObserver后

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_didAddObserver.png)

此时如果我们在lldb中执行：`p [alin methodForSelector:@selector(setGirlFriend:)]`可以发现`NSKVONotifying_Boy`的`setGirlFriend:`是一个`Foundation`中的函数：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_NSSetObjectValueAndNotify.png)

那么这个`Foundation`函数具体做了什么呢？由于Foundation框架没有开源，没法得知。但是我们可以从函数的命名可以猜测它至少做了两件事情：

* 设置被观察keyPath对应成员变量值
* 调用观察者实现的`observeValueForKeyPath…`这个方法将变化通知观察者。

熟悉kvo的伙伴应该会了解，其实我们可以通过调用如下两个方法手动触发kvo：

```objective-c
[alin willChangeValueForKey:@"girlFriend"];
[alin didChangeValueForKey:@"girlFriend"];
```

那么`_NSSetObjectValueAndNotify`大概的一个实现可能是：

```c
void _NSSetObjectValueAndNotify(id instance,NSString *keyPath) {
    [instance willChangeValueForKey:keyPath];
    [调用instance的super的set方法];
    [install didChangeValueForKey:keyPath];
}
```

可以大概验证一下上面的猜测,将Boy的实现代码更改如下：

```objective-c
@implementation Boy

- (void)setGirlFriend:(NSString *)girlFriend {
    NSLog(@"setGirlFriend: %@",girlFriend);
    _girlFriend = girlFriend;
}

- (void)willChangeValueForKey:(NSString *)key {
    NSLog(@"willChangeValueForKey: %@",key);
    [super willChangeValueForKey:key];
}

- (void)didChangeValueForKey:(NSString *)key {
    NSLog(@"didChangeValueForKey: %@",key);
    [super didChangeValueForKey:key];
}

@end
```

查看方法的调用可以发现还是非常符合我们的猜测的：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_verify_SetObjectValueAndNotify.png)

### class方法的困惑

如果读者对刚开始接触kvo的话，或许会对下面代码的打印结果产生疑问：

```objective-c
#import <Foundation/Foundation.h>
#import "Boy.h"
#import "Observer.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        Boy *alin = [[Boy alloc] init];
        NSLog(@"alinClass: %@",[alin class]);
        
        Observer *observer = [[Observer alloc] init];
        
        NSKeyValueObservingOptions options =
        NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
        
        [alin addObserver:observer forKeyPath:@"girlFriend" options:options context:nil];
        NSLog(@"addObserver_alinClass: %@",[alin class]);
        alin.girlFriend = @"Swift";
        
        [alin removeObserver:observer forKeyPath:@"girlFriend"];
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_class.png)

不是说好的执行`addObserver`后alin的变成`NSKVONotifying_Boy`类型的对象嘛？矛盾了啊。这里想告诉你的是Apple不仅仅在运行时改变了alin的isa指针，而且它为`NSKVONotifying_Boy`重写了class方法。在main.m中添加一个`printClassMethods`函数用来打印Class的方法：

```objective-c

void printClassMethods(Class cls) {
    unsigned int count;
    Method *methods = class_copyMethodList(cls, &count);
    
    NSMutableString *methodNames = [NSMutableString string];
    [methodNames appendFormat:@"%@ - ", cls];
    
    for (int i = 0; i < count; i++) {
        Method method = methods[i];
        
        NSString *methodName = NSStringFromSelector(method_getName(method));
        [methodNames appendString:methodName];
        [methodNames appendString:@" "];
    }
    
    NSLog(@"====================================");
    NSLog(@"%@", methodNames);
    NSLog(@"====================================");
    
    free(methods);
}
```

然后更改一下`main`函数的实现：

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        Boy *alin = [[Boy alloc] init];
        
        Observer *observer = [[Observer alloc] init];
        
        NSKeyValueObservingOptions options =
        NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew;
        
        [alin addObserver:observer forKeyPath:@"girlFriend" options:options context:nil];
        NSLog(@"addObserver_alinClass: %@",[alin class]);
        //=======================
        printClassMethods(objc_getClass(object_getClassName(alin)));
        //=======================
        alin.girlFriend = @"Swift";
        
        [alin removeObserver:observer forKeyPath:@"girlFriend"];
    }
    return 0;
}
```



![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_printClassMethods.png)

可以看到`NSKVONotifying_Boy`添加了如下4个方法的实现：

* setGirlFriend: 
* class   // 这里返回super的class
* dealloc 
* _isKVOA

到这里就已经很清晰地看到KVO的诡计了：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvo_methods.png)

### 推荐

1. 如果你想了解如何自己实现一个KVO推荐前往：[如何自己动手实现 KVO](http://tech.glowing.com/cn/implement-kvo/)
2. 如果你想要优雅地使用KVO推荐使用：[KVOController](https://github.com/facebook/KVOController)