---
layout: post
title: Objective-C:类和对象
subtitle: 
author: JackLin
date: 2018-01-19 23:27:15 +0800
---


>因为 ObjC 的 runtime 只能在 Mac OS 下才能编译，所以文章中的代码都是在 Mac OS，也就是 x86_64 架构下运行的,对于在 arm64 中运行的代码会可能会有些不同。

之前经常在其他的博客中曾经对 ObjC 底层的实现有一定的了解到Objective-C 对象都是 C 语言结构体，所有的对象都包含一个类型为 isa 的指针，那么你可能确实对 ObjC 的底层有所知，不过现在的 ObjC 对象的结构已经不是这样了。代替 isa 指针的是结构体 isa_t, 这个结构体中"包含"了当前对象指向的类的信息，这篇文章中会介绍一些关于这个变化的知识。

```oc
struct objc_object {
    isa_t isa;
};
```

当 ObjC 为一个对象分配内存，初始化实例变量后，在这些对象的实例变量的结构体中的第一个就是 isa。

```
所有继承自 NSObject 的类实例化后的对象都会包含一个类型为 isa_t 的结构体。
```

![有帮助的截图]({{ site.url }}/assets/postsImages/objc-isa-class-object.png)

>所有继承自 NSObject 的类实例化后的对象都会包含一个类型为 isa_t 的结构体。



从上图中可以看出，不只是实例会包含一个 isa 结构体，所有的类也有这么一个 isa。在 ObjC 中 Class 的定义也是一个名为 objc_class 的结构体，如下：

```oc 
struct objc_class : objc_object {
    isa_t isa;
    Class superclass;
    cache_t cache;
    class_data_bits_t bits;
};
```

到这里，我们就明白了：Objective-C 中类也是一个对象。

这个 isa 包含了什么呢？回答这个问题之前，要引入了另一个概念元类(meta class)，我们先了解一些关于元类的信息。

因为在 Objective-C 中，对象的方法(包括实例对象和类对象)并没有存储于对象的结构体中（因为在程序运行中可能一个类创建了很多实例对象，如果每一个对象都保存了自己能执行的方法，那么对内存的占用有极大的影响）。

当实例方法被调用时，它要通过自己持有的`isa`来查找对应的类，然后在查找到的类的`class_data_bits_t `结构体中查找对应方法的实现。同时，每一个 `objc_class` 也有一个指向自己的父类的指针 `super_class` 用来查找继承的方法。
>⚠️
>  关于isa_t和class_data_bits_t结构体里面的具体内容暂时不过分需要纠结，其不影响对类和对象方法查找大概过程的理解

类方法的实现又是如何查找并且调用的呢？这时，就需要引入元类来保证无论是类还是对象都能通过相同的机制查找方法的实现。

![有帮助的截图]({{ site.url }}/assets/postsImages/objc-isa-meta-class.png)

让每一个类的 isa 指向对应的元类，这样就达到了使类方法和实例方法的调用机制相同的目的：

* 实例方法调用时，通过对象的 isa 在类中获取方法的实现
* 类方法调用时，通过类的 isa 在元类中获取方法的实现
  下面这张图介绍了对象，类与元类之间的关系，已经觉得足够清晰了，所以不在赘述。

![有帮助的截图]({{ site.url }}/assets/postsImages/objc-isa-class-diagram.png)

下面结合题目看看:
#### 1. 下面的代码分别输出什么？

```oc
@implementation Son : Father
- (id)init {
    self = [super init];
    if (self) {
        NSLog(@"%@", NSStringFromClass([self class]));
        NSLog(@"%@", NSStringFromClass([super class]));
    }
    return self;
}
@end

```
题目来源[sunnyxx](http://blog.sunnyxx.com/2014/03/06/ios_exam_0_key/)

答案：都输出”Son”.  
解释：objc中super是编译器标示符，并不像self一样是一个对象，遇到向super发的方法时会转译成objc_msgSendSuper(...)，而参数中的对象还是self，于是从父类开始沿继承链寻找- class这个方法，从上面最后一个图可以看到最后都指向RootClass（即NSObject），所以最后在NSObject中找到（若无override，这个方法都是在NSObject中实现），此时，[self class]和[super class]已经等价了。

#### 2.下面的代码报错？警告？还是正常输出什么？

```oc
Father *father = [Father new];
BOOL b1 = [father responseToSelector:@selector(responseToSelector:)];
BOOL b2 = [Father responseToSelector:@selector(responseToSelector:)];
NSLog(@"%d, %d", b1, b2);
```

>
>答案：都输出”1”(YES).  
>解释：
>objc中：



* 不论是实例对象还是Class，都是id类型的对象（Class同样是对象）
* 实例对象的isa指向它的Class（储存所有减号方法）,Class对象的isa指向元类（储存所有加号方法）
* 向一个对象（id类型）发送消息时，都是从这个对象的isa指针指向的Class中寻找方法

回到题目，当像Father类发送一个实例方法（- responseToSelector）消息时：

1.会从它的isa，也就是Father元类对象中寻找，由于元类中的方法都是类方法，所以自然找不到

2.于是沿继承链去父类NSObject元类中寻找，依然没有

3.由于objc对这块的设计是，NSObject的元类的父类是NSObject类（也就是我们熟悉的NSObject类），其中有所有的实例方法，因此找到了- responseToSelector

>
>补充：NSObject类中的所有实例方法很可能都对应实现了一个类方法（至少从开源的代码中可以看出来），如+ resonseToSelector，但并非公开的API，如果真的是这样，上面到第2步就可以找到这个方法。

>再补充： 非NSObject的selector这样做无效。


#### 完







