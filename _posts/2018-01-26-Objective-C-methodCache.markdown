---
layout: post
title: Objective-C:方法缓存细节
subtitle: 
author: JackLin
date: 2018-01-26 00:03:39 +0800
---

### 前言

大家或许都了解到，Objectvie-C中的对象在一个方法被调用后会被添加到缓存中方便下次查找的时候直接可以从缓存中查找到以加快方法的查找时间。写这篇文章的起因是我对Objectvie-C中缓存的好奇

* 在方法被缓存后，objc_msgSend函数是否会进入lookUpImpOrForward然后在imp = cache_getImp(cls, sel)中获得函数的调用，还是在objc_msgSend内部做了其他处理不用进入lookUpImpOrForward即可完成。​


#### cache、objc_msgSend和lookUpImpOrForward

先来看看对象和类的大题结构

* 对象结构体的定义

```oc
struct objc_object {
    Class _Nonnull isa  OBJC_ISA_AVAILABILITY;
};
```

* 去掉一下函数后的objc_class类的结构体的定义

```oc
struct objc_class : objc_object {
    // Class ISA;
    Class superclass;
    cache_t cache;             // formerly cache pointer and vtable
    class_data_bits_t bits;
}
```

objc_class继承于objc_object，从上面两个结构体可以看到一个objc_class类的整体结构如下：

![有帮助的截图]({{ site.url }}/assets/postsImages/class_struct_cache_t.png)





在objc-runtime-new.h可以查找到cache_t的定义，如下:

```oc
struct cache_t {
    struct bucket_t *_buckets;
    mask_t _mask;
    mask_t _occupied;

public:
    struct bucket_t *buckets();
    mask_t mask();
    mask_t occupied();
    void incrementOccupied();
    void setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask);
    void initializeToEmpty();

    mask_t capacity();
    bool isConstantEmptyCache();
    bool canBeFreed();

    static size_t bytesForCapacity(uint32_t cap);
    static struct bucket_t * endMarker(struct bucket_t *b, uint32_t cap);

    void expand();
    void reallocate(mask_t oldCapacity, mask_t newCapacity);
    struct bucket_t * find(cache_key_t key, id receiver);
    static void bad_cache(id receiver, SEL sel, Class isa) __attribute__((noreturn));
};

```

_buckets是一个bucket_t结构体的数组，方法的imp就保存在bucket_t结构体中，imp和一个key配对绑定,同样可以在bjc-runtime-new.h找到bucket_t的定义，如下:

```oc
struct bucket_t {
private:
    cache_key_t _key;
    IMP _imp;
public:
    inline cache_key_t key() const { return _key; }
    inline IMP imp() const { return (IMP)_imp; }
    inline void setKey(cache_key_t newKey) { _key = newKey; }
    inline void setImp(IMP newImp) { _imp = newImp; }
    void set(cache_key_t newKey, IMP newImp);
};

```

结合上面的内容，整理结构图如下:

![有帮助的截图]({{ site.url }}/assets/postsImages/class_struct_cache_t_bucket_t.png)

了解整体的结构，还有方法缓存在方法在类结构中的位置后，下面就进行方法缓存的调试了，先创建一个Farter类，并且添加打印helloworld方法，随后在main.m中使用,具体代码如下:

```oc

#import "Farter.h"
@implementation Farter
- (void)helloworld {
    NSLog(@"helloworld");
}
@end

#import <Foundation/Foundation.h>
#import "Farter.h"
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [Farter new];
        [f helloworld];
        [f helloworld];
    return 0;
    }
}
```

我们知道，在执行第一个`[f helloworld]`,也就是在第一次调用objc_msgSend函数时会进入lookUpImpOrForward查找方法，找到方法的imp后想将其缓存最后调用方法的imp。我们让[f helloworld]先执行一次，在第二个helloworld调用前打下一个断点，使用`lldb`查看缓存中是否存放了`helloworld`：

![有帮助的截图]({{ site.url }}/assets/postsImages/get_cache_log.jpeg)

从打印的结果可以看到方法已经被缓存起来放到了cache_t 的buckets数组里面了，那么下次执行`[f helloworld]`还会进入lookUpImpOrForward里面吗？那么我们在lookUpImpOrForward里面添加一个断点看看是否有效:

![有帮助的截图]({{ site.url }}/assets/postsImages/get_cache_log_cahed.jpeg)

我们看到(控制台最后的红框)第二个hellworld的打印是没有进入lookUpImpOrForward的，现在进入的是`delloc`方法(左边的红框可以看到)，所以消息的缓存不是在lookUpImpOrForward内部的通过如下获得

```oc
    imp = cache_getImp(cls, sel);
    if (imp) goto done;
```

而是在此之前就已经获得缓存了的imp了，因此可以对第一个疑问得出推断：

> 对于缓存后的方法，在objc_msgSend内部做了其他处理不用进入lookUpImpOrForward即可完成查找和调用

这样我们重新执行一次，在第二次调用前将缓存中的`helloworld`的buckt_t清除掉验证一下。可以预测到清除后缓存后第二次调用会进入lookUpImpOrForward查找:

![有帮助的截图]({{ site.url }}/assets/postsImages/cached_debbuger01.jpeg)

> p $2[2] = $2[0]

如上，我先找到hellowrold对应的buckt_t，然后设置一个新的为空的buckt_t将其抹除,这样就没有了hellowrold的缓存。继续运行代码查看结果:



![有帮助的截图]({{ site.url }}/assets/postsImages/cached_debbuger02.jpeg)



和预测的一致，清除helloworld消息的缓存后，进入了lookUpImpOrForward进行消息查找，从左边的调用栈中看到`_objc_msgSend_uncached`调用，点击进入:

![有帮助的截图]({{ site.url }}/assets/postsImages/objc_msgSend_uncached01.jpeg)

是汇编实现的代码，注释中可以推测上面的是_objc_msgSend_uncached的入口，查看文件里面的代码会发现:

![有帮助的截图]({{ site.url }}/assets/postsImages/objc_msgSend_uncached02.jpeg)

从上面的汇编代码和注释可以看出`objc_msgSend`中进行了如下处理:

>* 检查receiver是否为空
>* 从缓存中查找selector的imp，找到就直接调用imp
>* 缓存中没有找到就调用_objc_msgSend_uncached，去class的方法列表里面找

结合[Objective-C的消息机制]，整理出完整objc_msgSend函数处理的逻辑图:

![有帮助的截图]({{ site.url }}/assets/postsImages/message_find_all.png)

> 上面的逻辑图对方法决议`_class_resolveMethod`这一步省略了是因为`_class_resolveMethod`就是根据当前类是否为metaClass再次调用lookUpImpOrForward查找类是否实现了`+resolveClassMethod`  or`+resolveInstanceMethod`.如果实现了就向当前类是否发送实现的消息调用实现的方法。在被调用的实现中通常会给`sel`添加一个`imp`,而且imp会被缓存起来。这样再次递归`lookUpImpOrForward`就会查在方法缓存中找到需要的`imp`.从而_class_resolveMethod没有跳出`lookUpImpOrForward`.而forward一般不同，它将消息转发到其他类，从而可以认为进入了另外的一个`objc_msgSend`。

这里就不对消息机制的过程进行更多的说明了，已经严重偏离主题，回到刚刚消息缓存这里。上面我们将helloworld的`bucket_t`改为空的,从而进入了lookUpImpOrForward再次查找。不过，还有一个更好玩的地方，如果我们只是替换了helloworld的`bucket_t`中的_imp会如何呢？一起来play 一下:

```oc
#import <Foundation/Foundation.h>
#import "Farter.h"
#import <objc/message.h>

void helloChanged() {
   NSLog(@"changed_imp helloworld");
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [Farter new];
        [f helloworld];
        [f helloworld];
    return 0;
    }
}
```

在 main.m里面添加一个`helloChanged`函数，然后在第二个[f helloworld]调用前到一个断点，随后将helloChanged函数的替换掉类结构缓存中的helloworld方法的实现:

![有帮助的截图]({{ site.url }}/assets/postsImages/cached_change_imp.jpeg)

然后继续运行代码:

![有帮助的截图]({{ site.url }}/assets/postsImages/cached_change_imp_end.jpeg)

可以看到，在没有改变helloworld的实现，同样是发送helloworld消息,但是第二次调用了helloChanged。这是因为在 **objc_msgSend** 的消息发送链路中，使用错误的缓存实现 `helloChanged` 拦截了helloworld实现的查找，打印出了 `changed_imp helloworld`。



有时候你会发现，原来已经缓存过的方法实现会被清除，需要再次进入lookUpImpOrForward，这是因为在将一个方法实现加入类的缓存中的时候会先检查缓存中的内容若大于容量的 `3/4` 就会扩充缓存，使缓存的大小翻倍。**在缓存翻倍的过程中，当前类全部的缓存都会被清空，Objective-C 出于性能的考虑不会将原有缓存的 `bucket_t` 拷贝到新初始化的内存中。**详细的操作在 `cache_fill_nolock` 方法中:

```oc
static void cache_fill_nolock(Class cls, SEL sel, IMP imp, id receiver)
{
    if (!cls->isInitialized()) return;
    if (cache_getImp(cls, sel)) return;

    cache_t *cache = getCache(cls);
    cache_key_t key = getKey(sel);

    mask_t newOccupied = cache->occupied() + 1;
    mask_t capacity = cache->capacity();
    if (cache->isConstantEmptyCache()) {
        cache->reallocate(capacity, capacity ?: INIT_CACHE_SIZE);
    } else if (newOccupied <= capacity / 4 * 3) {

    } else {
        cache->expand();
    }

    bucket_t *bucket = cache->find(key, receiver);
    if (bucket->key() == 0) cache->incrementOccupied();
    bucket->set(key, imp);
}
```

#### the end ,code night

