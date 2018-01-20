---
layout: post
title: load和initialize
subtitle: 
author: JackLin
date: 2017-01-19 14:32:00 +0800
---
#### +(void)load 和 +(void)initialize


在iOS的开发中，绝大多数时候你都不需要关心一个类是怎么被加载进内存的。这里面 runtime linker 在你的代码还没跑起来之前就已经做了很多复杂的工作。

对于大多类来说，知道这一点就已经相当足够了。但是，有一些类可能需要做一些特殊的准备工作。比如初始化一个全局的表，从 UserDefaults 里面读取配置并缓存起来，又或者做一些其他的准备工作。

ObjC 提供了两种方法来实现这些事情：
>
>+ initialize 
+ load


请看如下代码:


``` 
#import "ALSuper.h"


@implementation ALSuper (ALSuperMe)

+ (void)load {
    NSLog(@"ALSuperMe load: %@",NSStringFromClass(self));
}

+ (void)initialize {
    NSLog(@"ALSuperMe initialize: %@",NSStringFromClass(self));
}

@end


@implementation ALSuper (ALSuperYou)
+ (void)load {
    NSLog(@"ALSuperYou load: %@",NSStringFromClass(self));
}

+ (void)initialize {
    NSLog(@"ALSuper_initialize");
    NSLog(@"ALSuperYou initialize: %@",NSStringFromClass(self));
}


@end

@implementation ALSuper

+ (void)load {
    NSLog(@"load: %@",NSStringFromClass(self));
}

+ (void)initialize {
    NSLog(@"initialize: %@",NSStringFromClass(self));
}

@end


``` 
```
 #import "ALSub.h"

@implementation ALSub

+ (void)load {
    NSLog(@"hello load: %@",NSStringFromClass(self));
}

//+ (void)initialize {
//    NSLog(@"hello initialize: %@",NSStringFromClass(self));
//}

@end
``` 


```
#import <Foundation/Foundation.h>
#import "ALSuper.h"

@interface ALSub :ALSuper

@end

```

执行下面的代码打印

```
 ALSub *alSub = [[ALSub alloc] init];
```
打印结果:

```
2018-01-19 14:10:01.688191+0800 BuDejie[25798:1713046] load: ALSuper
2018-01-19 14:10:01.689182+0800 BuDejie[25798:1713046] load: ALSub
2018-01-19 14:10:01.690664+0800 BuDejie[25798:1713046] ALSuperMe load: ALSuper
2018-01-19 14:10:01.691203+0800 BuDejie[25798:1713046] ALSuperYou load: ALSuper
2018-01-19 14:10:01.929028+0800 BuDejie[25798:1713046] ALSuper_initialize
2018-01-19 14:10:01.929192+0800 BuDejie[25798:1713046] ALSuperYou initialize: ALSuper
2018-01-19 14:10:01.929296+0800 BuDejie[25798:1713046] ALSuper_initialize
2018-01-19 14:10:01.929387+0800 BuDejie[25798:1713046] ALSuperYou initialize: ALSub
```

##### +(void)load:   
在加载这个类的时候调用，只会被调用一次。 
+load 这个方法有一个有意思的特性，就是 runtime 会把所有 category 里面实现了 +load 的方法全部调一遍。也就是说如果你在多个 category 里面都实现了 +load 方法，这些方法都会被调用一次，从下面打印可以看到ALSuperMe、ALSuperYou两个category中的+load 方法都被调用了。

> 你过去可能会听说过，对于 load 方法的调用顺序有两条规则：

>1.父类先于子类调用
>
2.类先于分类调用

```
2018-01-19 14:10:01.688191+0800 BuDejie[25798:1713046] load: ALSuper
2018-01-19 14:10:01.689182+0800 BuDejie[25798:1713046] load: ALSub
2018-01-19 14:10:01.690664+0800 BuDejie[25798:1713046] ALSuperMe load: ALSuper
2018-01-19 14:10:01.691203+0800 BuDejie[25798:1713046] ALSuperYou load: ALSuper

```
这种设计可能跟你认识到 category 的机制完全相反，不过你要知道 +load 方法不是一个普通的方法。这个特性决定了 +load 是一个干坏事的绝佳场所，比如 swizzling。

##### +(void)initialize:

```
2018-01-19 14:10:01.929192+0800 BuDejie[25798:1713046] ALSuperYou initialize: ALSuper
2018-01-19 14:10:01.929387+0800 BuDejie[25798:1713046] ALSuperYou initialize: ALSub
```
从上面两条结果可以看出，如果分类有实现+initialize 方法则只调用分类的实现，相比而言，就要正常的多了,，通常是一个更好的安置代码的地方。+initialize 有意思的地方在于它会很晚才被调用，甚至它有可能完全不会被调用。当一个类被加载的时候，+initialize 不会被调用，当一个消息发送给这个类的时候（ObjC 的方法调用都是通过 runtime 的消息机制，objc_sendMsg 方法），runtime 就会检查这个方法有没有被调用过，如果没有就调用之。跟 +load 方法一样，+initialize 会先调用这个类所有的父类，最后才调到自己的 +initialize 方法。ObjC 做 selector 实现的检查，如果当前类没有实现这个方法，那么父类的方法就会被调用。如果你有没实现 +initialize 的子类，你的代码就会被调用两次。就算你没有任何子类，Apple 的 KVO 也会动态创建没有实现 +initialize 的子类。如ALSuper_initialize打印了两次，证明ALSuper这个category的initialize被执行了两次,所以在有需要的地方要注意类型检查。

##### 使用建议

ObjC 提供了两种自动运行类初始化代码的方法。+load 方法保证了会在 class 被加载的时候调用，这个时机很早，所以对于需要很早被执行的代码来说是很有用的。但是在这个时机跑的代码也可以是很危险的，毕竟这个时候的环境比较恶劣。

由于 +initialize 方法是 lazy 触发的，所以对于初始化设置的环境就要友好得多。只要不是在类接收第一条消息之前一定要做的事情，都可以在这个方法里面做。