---
layout: post
title:  Objective-C:AssociatedObject
subtitle: 
author: JackLin
date: 2018-01-30 00:29:44 +0800
---

### 前言

我们在 iOS 开发中经常需要使用分类（Category），为已经存在的类添加属性的需求，但是使用 `@property` 并不能在分类中**正确**创建实例变量和存取方法，同时我们也不能直接在（Category)添加实例变量。不过，通过 Objective-C 运行时中的关联对象，也就是 Associated Object，我们可以实现上述需求。

**这篇文章试图解决两个问题:**

> 1. 为什么要使用关联对象?
> 2. 如何使用关联对象？

### 为什么要使用关联对象?

在前言其实已经可以看出，要使用关联对象是基于以下两个点:

>1. 有时候我们会遇到需要为已有的类(我们无法修改的类)添加实例变量
>2. category分类无法正确地创建实例变量和存取方法



我们知道通常在类或者Extension中使用@property时Xcode会帮我为对应的属性添加setter和getter方法，并且创建一个实例变量:

```oc
#import <Foundation/Foundation.h>

@interface MyObject:NSObject

@property (nonatomic, copy) NSString *alinProperty;

@end
```

Everything is good,上面的代码相当于如下代码:

```oc
#import <Foundation/Foundation.h>

@interface MyObject:NSObject
{
 NSString *_alinProperty;
}

@end

#import "MyObject.h"

@implementation MyObject

- (NSString *)alinProperty {
    return _alinProperty;
}

- (void)setAlinProperty:(NSString *)alinProperty {
    _alinProperty = alinProperty;
}

@end
```

这些代码都是编译器为我们生成的，虽然你看不到它，但是它确实在这里，我们既然可以在类中使用 `@property` 生成一个属性。但是要为NSObject添加一个alinProperty属性，我们使用category去实现：

```oc
#import <Foundation/Foundation.h>

@interface NSObject (ALin)

@property (nonatomic, copy) NSString *alinProperty;

@end
```

Bad news: 

![有帮助的截图]({{ site.url }}/assets/postsImages/associatedObjec_warringt.png)

警告告诉我们` alinProperty `属性的存取方法需要自己手动去实现，或者使用 `@dynamic` 在运行时实现这些方法。换句话说，编译器告诉你它没有针对分类中的 `@property` 为我们生成实例变量以及存取方法，而需要我们手动实现。使用@dynamic相当于告诉编译器你要自己实现，这样编译器就不会警告你了。

### 如何使用关联对象？

Associated Object 属于runtime的内容，相关的API定义的<objc/runtime.h>中，所以要使用就需要在文件中导入。最后将NSObject (ALin)实现添加如下代码:

```oc
#import "NSObject+ALin.h"
#import <objc/runtime.h>

@implementation NSObject (ALin)

- (void)setAlinProperty:(NSString *)alinProperty {
    objc_setAssociatedObject(self, @selector(alinProperty), alinProperty, OBJC_ASSOCIATION_COPY);
}

- (NSString *)alinProperty {
   return objc_getAssociatedObject(self, @selector(alinProperty));
}

@end
```

Everything is good,编译通过，警告消除! 去主函数测试使用刚刚为NSObject添加的属性看看能否正常有效：

```oc
#import <Foundation/Foundation.h>
#import "NSObject+ALin.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSObject *ob = [NSObject new];
        ob.alinProperty = @"hello alin";
        NSLog(@"%@",ob.alinProperty);
    return 0;
    }
}

```

Perfect, `ob.alinProperty` 和 `ob.alinProperty = @"hello alin"`一切正常。



### the end





