---
layout: post
title: Objective-C:+load
subtitle: 
author: JackLin
date: 2018-01-28 14:34:12 +0800
---

你过去可能和笔者一样听说过，对于 `load` 方法的调用顺序有三条规则：

1. `main` 函数之前由Objectvie-C的runtime调用
2. 父类先于子类调用
3. 类先于分类调用

首先写点代码测试一下上面的3条规则:

```oc

@implementation ALin
+ (void) load {
    NSLog(@"Alin ===== load");
}
@end

@implementation ALin (hi)
+ (void)load {
    NSLog(@"ALin+hi === load");
}
@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSLog(@"main");
    return 0;
    }
}
```

在main.n函数中仅仅是打印了一个main，没有调用任何的ALin这个类的任何+load,但是还是执行了ALin类实现的和ALin+hi分类中的+load.验证上面的三条规则确实没错.

![有帮助的截图]({{ site.url }}/assets/postsImages/load_ALin.png)

可是` load` 方法是究竟如何被调用的，`+ load` 方法为什么会有这种调用顺序呢？这些问题值得考究,首先来通过 `load` 方法的调用栈，分析一下它到底是如何被调用的。

> 因为 ObjC 的 runtime 只能在 Mac OS 下才能编译，所以文章中的代码都是在 Mac OS，也就是 x86_64 架构下运行的，你可以在[objc-runtime](https://github.com/RetVal/objc-runtime)clone 整个仓库来进行调试。

在+ load中添加一个断点

![有帮助的截图]({{ site.url }}/assets/postsImages/load_stop_0.jpg)

左侧的调用栈很清楚的看到哪些方法被调用了：

> ```
> 0  +[ALin load]
> 1  call_class_loads()
> 2  call_load_methods
> 3  load_images
> 4  dyld::notifySingle(dyld_image_states, ImageLoader const*)
> 11 _dyld_start
> ```

[dyld](https://developer.apple.com/library/ios/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html) 是 the dynamic link editor 的缩写，它是苹果的*动态链接器*,又它来加载程序的可执行文件

每当有新的镜像加载之后，都会执行 `3 load_images` 方法进行回调，在进入`load_images`函数打下断点，查`load_images`看的调用栈。

![有帮助的截图]({{ site.url }}/assets/postsImages/load_load_images.jpg)

看了dyld_start之后进入了_objc_init:

```oc
void _objc_init(void)
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    environ_init();
    tls_init();
    static_init();
    lock_init();
    exception_init();
    _dyld_objc_notify_register(&map_images, load_images, unmap_image);
}
```

在这个函数调用_dyld_objc_notify_register注册了一个监听dyld加载image的函数，进入map_images:

```oc

map_images(unsigned count, const char * const paths[],
           const struct mach_header * const mhdrs[])
{
    rwlock_writer_t lock(runtimeLock);
    return map_images_nolock(count, paths, mhdrs);
}

```

随后发现最调用了_read_images(header_info **hList, uint32_t hCount, int totalClasses, int unoptimizedTotalClasses)去读取并检查所有的文件的内容，_read_images函数特别长，检查所有的header_info信息，针对不同的内容信息进行对应的处理，这里取一部分比较关心的内容:

### Realize non-lazy classes

如果发现有+load方法的类的定义，或者static instances就会调用realizeClass(cls);对类进行初始化:

> realizeClass(cls);是关注点

```oc
    // Realize non-lazy classes (for +load methods and static instances)
    for (EACH_HEADER) {
        classref_t *classlist =
            _getObjc2NonlazyClassList(hi, &count);
        for (i = 0; i < count; i++) {
            Class cls = remapClass(classlist[i]);
            if (!cls) continue;
#if TARGET_OS_SIMULATOR
            if (cls->cache._buckets == (void*)&_objc_empty_cache  &&
                (cls->cache._mask  ||  cls->cache._occupied))
            {
                cls->cache._mask = 0;
                cls->cache._occupied = 0;
            }
            if (cls->ISA()->cache._buckets == (void*)&_objc_empty_cache  &&
                (cls->ISA()->cache._mask  ||  cls->ISA()->cache._occupied))
            {
                cls->ISA()->cache._mask = 0;
                cls->ISA()->cache._occupied = 0;
            }
#endif
            realizeClass(cls);
        }
    }

```

### Discover categories

如果发现是categories内容，通过_getObjc2CategoryList函数获得所有的category_t，如果类已经实现那么就会调用`remethodizeClass`将categories的内容添加到类中,	这里`addUnattachedCategoryForClass`是把类和category做一个关联映射，作用就是将添加是把类和category做一个关联映射到一个table里面管理，方便后面remethodizeClass调用的时候可以使用cls获取所有的unattached的Category.

> remethodizeClass(cls)是关注点

```oc

  // Discover categories.
    for (EACH_HEADER) {
        category_t **catlist =
            _getObjc2CategoryList(hi, &count);
        bool hasClassProperties = hi->info()->hasCategoryClassProperties();

        for (i = 0; i < count; i++) {
            category_t *cat = catlist[i];
            Class cls = remapClass(cat->cls);
            if (!cls) {
                catlist[i] = nil;
                }
                continue;
            }
            // categories中的实例方法添加到类方法列表
            bool classExists = NO;
            if (cat->instanceMethods ||  cat->protocols
                ||  cat->instanceProperties)
            {
                addUnattachedCategoryForClass(cat, cls, hi);
                if (cls->isRealized()) {  // 
                    remethodizeClass(cls);
                    classExists = YES;
                }
            }

			// categories中的类方法添加到元类方法列表
            if (cat->classMethods  ||  cat->protocols
                ||  (hasClassProperties && cat->_classProperties))
            {
            	// 是把类和category做一个关联映射，作用就是将添加是把类和category做一个关联映射到一个 				 //Table里面管理，方便后面remethodizeClass调用的时候可以使用cls获取所有的Unattached				的Category
                addUnattachedCategoryForClass(cat, cls->ISA(), hi);
                if (cls->ISA()->isRealized()) {
                    remethodizeClass(cls->ISA());
                }
            }
        }
    }
```

可以发现，代码运行到这里的时候，如果类中有+load方法，那么该类在这里就已经将类实现属性，协议和方法添加到类的列表了，而且和当前类相关的categories中添加的属性，协议和方法也已经加载到类的列表中。既然发现了这点，那就顺便验证一下，在ALin的实现和它的分类中添加一个`sayTo`方法，所有代码如下:

```oc
@implementation ALin
+ (void) load {
    NSLog(@"Alin ===== load");
}
- (void)sayTo:(NSString *)name {
    NSLog(@"ALin say hi to %@",name);
}
@end

@implementation ALin (hi)
+ (void)load {
    NSLog(@"ALin+hi === load");
}
- (void)sayTo:(NSString *)name {
    NSLog(@"ALin+hi say hi to %@",name);
}
@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSLog(@"main");
    return 0;
    }
}
```

随后在 _read_images函数的discover categories中添加一个条件断点，让程序在为ALin类`remethodizeClass`后停下,如图:

![有帮助的截图]({{ site.url }}/assets/postsImages/category_check_condition.png)

在lldb中查看类的相关信息

![有帮助的截图]({{ site.url }}/assets/postsImages/load_category_ddlb_0.jpg)

从打印的结果可以发现，到这里类的实现已经完成，类和它的category的内容都已经正确地install到类的实现中.而且你可以发现category中和class中相同的方法`sayTo:`同时存在`methods`中,只是category中的方法会在`methods`比较前面的位置，不是大家常说的**category的方法覆盖类的同名方法**，关于`category`更详细的内容我后面计划单独写一篇文章。

### load_images

在dyld的所有被mapped的image的镜像在_read_images完成后会回调`load_images`处理 `image`中包含的所有的+load方法，包括class和category的+load方法:

```oc

void load_images(const char *path __unused, const struct mach_header *mh)
{
	// 如果镜像中没有+load直接返回
    if (!hasLoadMethods((const headerType *)mh)) return;
    recursive_mutex_locker_t lock(loadMethodLock);
    { //准备+load
        rwlock_writer_t lock2(runtimeLock);
        prepare_load_methods((const headerType *)mh);
    }
	 //调用+load
    call_load_methods();
}

```

> 1.先判断image中是否有+load，没有就直接返回
>
> 2.调用 `prepare_load_methods` 对 `load` 方法的调用进行准备
>
> 3.调用`call_load_methods()`执行上面准备好的所有的`load`方法

### prepare_load_methods

```oc
void prepare_load_methods(const headerType *mhdr)
{
    size_t count, i;
    runtimeLock.assertWriting();
    classref_t *classlist = 
        _getObjc2NonlazyClassList(mhdr, &count);
    for (i = 0; i < count; i++) {
        schedule_class_load(remapClass(classlist[i]));
    }

    category_t **categorylist = _getObjc2NonlazyCategoryList(mhdr, &count);
    for (i = 0; i < count; i++) {
        category_t *cat = categorylist[i];
        Class cls = remapClass(cat->cls);
        if (!cls) continue; 
        realizeClass(cls);
        assert(cls->ISA()->isRealized());
        add_category_to_loadable_list(cat);
    }
}
```

可以看到`prepare_load_methods`分两部分，

* 对类中的+load做准备，通过 `_getObjc2NonlazyClassList` 获取所有的包含`+load`方法的类的列表之后，会通过 `remapClass` 获取类对应的指针，最后调用 `schedule_class_load` **递归地安排当前类和没有调用 + load 父类**进入列表。
* 对Category中的+load做准备，通过` _getObjc2NonlazyCategoryList` 获取所有的包含`+load`方法的Category的列表之后，会通过 `remapClass` 获取类对应的指针，因为在_read_image中仅仅对有+load的类作了实现，这里需要先调用realizeClass来实现类，最后调用 `add_category_to_loadable_list`,将所有在cateory中没有被调用过的+load,加入待调用列表.



#### schedule_class_load

```oc

static void schedule_class_load(Class cls)
{
    if (!cls) return;
    assert(cls->isRealized()); 
    if (cls->data()->flags & RW_LOADED) return;
    //递归到最后的super
    schedule_class_load(cls->superclass);
    
    add_class_to_loadable_list(cls); 
    cls->setInfo(RW_LOADED); 
}
```

> **schedule_class_load(cls->superclass)递归，保证add_class_to_loadable_list(cls)一定是先父类后子类**





### add_class_to_loadable_list

```oc
void add_class_to_loadable_list(Class cls)
{
    IMP method;

    loadMethodLock.assertLocked();

    method = cls->getLoadMethod();
    if (!method) return; 
    // 看看是否需要重新为列表分配空间
    if (loadable_classes_used == loadable_classes_allocated) {
        loadable_classes_allocated = loadable_classes_allocated*2 + 16;
        loadable_classes = (struct loadable_class *)
            realloc(loadable_classes,
                              loadable_classes_allocated *
                              sizeof(struct loadable_class));
    }
    
    // 一个loadable_class添加到loadable_classes数组里面
    loadable_classes[loadable_classes_used].cls = cls;
    loadable_classes[loadable_classes_used].method = method;
    loadable_classes_used++;
}
```

> `add_class_to_loadable_list`方法刚被调用时：
>
> 1. 会从 `class` 中获取 `+load` 方法： `method = cls->getLoadMethod();`
> 2. 判断当前 `loadable_classes` 这个数组是否已经被全部占用了：`loadable_classes_used == loadable_classes_allocated`
> 3. 在当前数组的基础上扩大数组的大小：`realloc`
> 4. 把传入的 `class` 以及对应的`+load`方法的实现加到列表中

### add_category_to_loadable_list

```oc

void add_category_to_loadable_list(Category cat)
{
    IMP method;
    loadMethodLock.assertLocked();
    method = _category_getLoadMethod(cat);
    if (!method) return;
    if (loadable_categories_used == loadable_categories_allocated) {
        loadable_categories_allocated = loadable_categories_allocated*2 + 16;
        loadable_categories = (struct loadable_category *)
            realloc(loadable_categories,
                              loadable_categories_allocated *
                              sizeof(struct loadable_category));
    }

    loadable_categories[loadable_categories_used].cat = cat;
    loadable_categories[loadable_categories_used].method = method;
    loadable_categories_used++;
}
```

>`add_category_to_loadable_list`方法与`add_class_to_loadable_list`实现几乎完全相同：

1. 会从 `category` 中获取 `+load` 方法： `method = _category_getLoadMethod(cat);`
2. 判断当前 `loadable_classes` 这个数组是否已经被全部占用了：`loadable_categories_used == loadable_categories_allocated`
3. 在当前数组的基础上扩大数组的大小：`realloc`
4. 把传入的 `cat` 以及对应的`+load方`法的实现加到列表中

* 到这里`prepare_load_methods`的处理已经结束，获得了两个列表

> 1. loadable_classes，按照先父类后之类的顺序存放了所有class中的指针和class中添加的+load的方法imp
> 2. loadable_categories,存放了所有category的指针和category中添加的+load方法的imp

### call_load_methods

经过了前面的`prepare_load_methods`一切准备就绪，开始调用前准备好的所有的+load

```oc
void call_load_methods(void)
{
    static bool loading = NO;
    bool more_categories;

    loadMethodLock.assertLocked();
    if (loading) return;
    loading = YES;
    void *pool = objc_autoreleasePoolPush();

    do {
        // 1. 重复调用 class +loads 直到没有
        while (loadable_classes_used > 0) {
            call_class_loads();
        }
        // 2. 调用一次 category +loads
        more_categories = call_category_loads();
        
    } while (loadable_classes_used > 0  ||  more_categories);
    objc_autoreleasePoolPop(pool);
    loading = NO;
}
```

> 调用顺序如下：
>
> 1. 不停调用`call_class_loads`加载调用类的 `+ load` 方法，直到 `loadable_classes` 为空
> 2. 调用**一次** `call_category_loads` 加载分类
> 3. 如果有 `loadable_classes` 或者更多的分类，继续调用 `load` 方法
> 4. 在` call_class_loads`和` call_category_loads`中都是先进列表的load方法先被调用

** **

到此，load方法的准备和调用过程已经清晰了:

第1条规则是由于在加载完所有的runtime运行需要的文件后才会执行mian函数：

```oc
libdyld.dylib`star -> maim
```

第2条规则是由于 `schedule_class_load` 有如下的实现：

```oc
static void schedule_class_load(Class cls)
{
	···
    schedule_class_load(cls->superclass);
    add_class_to_loadable_list(cls);
    ···
}
```

> 这里通过这行代码 `schedule_class_load(cls->superclass)` 总是能够保证没有调用 `load` 方法的父类先于子类加入 `loadable_classes` 数组，从而确保其调用顺序的正确性。

第3条规则主要在 `call_load_methods` 中实现:

```oc
do {
    while (loadable_classes_used > 0) {
        call_class_loads();
    }
    more_categories = call_category_loads();
} while (loadable_classes_used > 0  ||  more_categories);
```

> 上面的 `do while` 语句先确保类的 `load` 方法会先于分类调用。

再一次Review一下:

>1. `main` 函数之前调用
>2. 父类先于子类调用
>3. 类先于分类调用



### the end , code night.







