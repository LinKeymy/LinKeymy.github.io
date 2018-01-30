---
layout: post
title: Objective-C:Category
subtitle: 
author: JackLin
date: 2018-01-29 00:27:23 +0800

---

### 前言

我们在编写任何app时，不要寄希望于需求是固定的、不变化的，这是不现实也是不科学的想法。同样，无论一个类设计的多么完美，在未来的需求演进中，或多或少可能都要为其增加或者修改一些功能，那怎么扩展已有的类呢？一般而言，继承和组合是不错的选择，但是有时这样又显得耦合或者杀鸡用了牛刀。但是在Objective-C 2.0中，提供了category这个语言特性，可以动态地为已有类添加新行为。如今category已经遍布于Objective-C代码的各个角落，从Apple官方的framework到各个开源框架，从功能繁复的大型APP到简单的应用，catagory无处不在。

**Tips **

> 1. 因为 ObjC 的 runtime 只能在 Mac OS 下才能编译，所以文章中的代码都是在 Mac OS，也就是 x86_64 架构下运行的，你可以在[这里clone ](https://github.com/RetVal/objc-runtime)整个仓库来进行对应的调试。
> 2. 由于篇幅原因或者部分不影响主要内容理解，这里会使用······代表忽略的部分代码，感兴趣的可以直接查看完整的源码(在1中clone下来的)

###  Category的使用

- 把类的实现分开在几个不同的文件里面

  > 1. 可以减少单个文件的体积 
  > 2. 可以把不同的功能组织到不同的category里 
  > 3. 可以由多个开发者共同完成一个类 ,方便多人开发
  > 4. 可以按需加载想要的category 

- 声明私有方法

  > 在category中声明的方法，只在类的实现文件中导入，这样外界就无法访问了

  > ```oc
  > #import "ALin+hi.h"
  >
  > @implementation ALin (hi)
  > - (void)sayTo:(NSString *)name {
  >     NSLog(@"say hi to %@",name);
  > }
  > @end
  > ```
  >
  > ```oc
  >
  > #import "ALin.h"
  > #import "ALin+hi.h"
  > @implementation ALin
  >
  > - (void)callSayTo:(NSString *)name {
  >     [self sayTo:name];
  > }
  >
  > @end
  > ```
  >
  > 上面的`sayTo:`只能在ALin内使用


- 模拟多继承

  一般情况下，多继承就是为了让一个子类同时拥有多个父类的功能。这样的需求可以使用category来完成。

  我们为一个类ALin添加两个category: CategoryA和CategoryB，这样ALin同时拥有了CategoryA和CategoryB的功能，而不需要继承和组合就可以实现。这样就模拟了多继承的主要功能：

  >```oc
  >#import "ALin+hi.h"
  >
  >@implementation ALin (Hi)
  >
  >- (void)sayTo:(NSString *)name {
  >    NSLog(@"ALin say hi to %@",name);
  >}
  >
  >@end
  >
  >#import "ALin+hi.h"
  >
  >@implementation ALin (Hi)
  >- (void)sayTo:(NSString *)name {
  >    NSLog(@"ALin say hi to %@",name);
  >}
  >
  >@end
  >#import "ALin+Bye.h"
  >
  >@implementation ALin (Bye)
  >
  >- (void)bye:(NSString *)name {
  >    NSLog(@"ALin say bye to %@",name);
  >}
  >
  >@end
  >```
  >
  >上面ALin不需要自己实现就具有`sayTo:`和`bye:`的能力，就好像是`ALin (Hi)`和`ALin (Bye)`中继承过来的一样.（仅仅是好像哦）

- 把framework的私有方法公开 

  > 对framework的类添加扩展，然后在扩展中添加需要公开私有方法的声明即可。
  >
  > 可见，在 ObjC 的 runtime 的消息机制小，没有所谓的私有方法啊。

### Category和Extension

Extension看起来很像一个匿名的Category，但是extension和有名字的Category几乎完全是两个东西。

 **Extension在编译期决议**    

Extension就是类的一部分，在编译期和头文件里的@interface以及实现文件里的@implement一起形成一个完整的类，它伴随类的产生而产生，亦随之一起消亡。extension一般用来隐藏类的私有信息，你必须有一个类的源码才能为一个类添加extension，所以你无法为系统的类比如NSString添加extension

**Category在运行期决议**     

Category中的所有内容都是在运行期实现一个类的时候调用`methodizeClass`或者`remethodizeClass`被install到类结构中的，就category和extension的区别来看，我们可以推导出一个明显的事实，extension可以添加实例变量，而category是无法添加实例变量的（因为在运行期，对象的内存布局已经确定，如果添加实例变量就会破坏类的内部布局，这对编译型语言来说是灾难性的）

### Category和Protocol

众所周知，OC是单继承，新出的Swift也是单继承。那么在iOS开发中，我们怎么实现类似多继承的关系？事实上，我们可以利用OC中的Category和Protocol来达到多继承的效果。那么Category和Protocol有什么区别呢？

**Protocol只是声明一套接口，并不能提供具体实现，变相的也算是一种抽象基类的实现方式（OC本身语法并不支持抽象基类）。**

Protocol只能提供一套公用的接口声明，并不能提供具体实现，它的行为是，我只负责声明，而不管谁去实现，去如何实现。这样的话，我定义一套接口，可以使任意的类都用不同的方式去实现接口中的方法，就是为遵守了protocol的类提供了一些额外访问这个类的一些接口，像delegate和dataSource用protocol实现是最好的。

**Category可以为已有的类提供额外的接口和具体的实现。**

Category是对一个功能完备的类的一种补充、扩展，就像一个东西基本功能都完成了，可以用category为这个类添加不同的组件，使得这个类能够适应不同情况的需求（但是这些不同需求最核心的需求要一致）。当然，当某个类非常大的时候，使用category可以按照不同的功能将类的实现分在不同的模块中。还有，虽然category可以访问已有类的实例变量，但不能创建新的实例变量，如果要创建新的实例变量，请使用继承。

### Category和+load

在类和category中都可以有+load方法，关于load调用顺序的详细理解这里就不再从源码里面看了，我单独写过一篇文章来探究[+load](http://onevlin.com/2018/01/Objective-C-load/)方法。+load方法的调用有三条基本规则:

> 1. `main` 函数之前调用
> 2. 父类先于子类调用
> 3. 类先于分类调用

可是，如果多个分类中都添加了+load方法呢？调用顺序会怎么样，从[+load](http://onevlin.com/2018/01/Objective-C-load/)可以了解到在加载可执行文件的时候准备category的+load方法时，是按照先dicover的先被放到loadable_categories里面的顺序，也就是先读到先加入loadable_categories，在call_category_loads调用category的+load方法时是按照FIFO的顺序调用。所以category中的+load方法的调用和其载入顺序有关，而载入顺序又依赖于编译的顺序:

```oc
@implementation ALin (Hi)

+ (void)load {
    NSLog(@"ALin (Hi) + load");
}
@end

@implementation ALin (Bye)

+ (void)load {
    NSLog(@"ALin (Bye) + load");
}

@end
```

上面在ALin的两个分类中都添加了+load，接下来设置两个不同的编译顺序运行查看打印结果，对比执行的顺序:

* 先编译ALin+Bye.m

![有帮助的截图]({{ site.url }}/assets/postsImages/category_load_build.jpg)

* 先编译ALin+hi.m

  ![有帮助的截图]({{ site.url }}/assets/postsImages/category_load_build01.jpg)

  ​

可一发现确实调用顺序和编译顺序是意义对应的关系，所以如果要为+load方法添加第四条规则的话:

> 分类中的+load调用的是先编译先调用

### Category的结构和权限

我们知道，所有的OC类和对象，在runtime层都是用struct表示的，category也不例外，在runtime层，category用结构体category_t（在objc-runtime-new.h中可以找到此定义），它包含了

```oc
struct category_t {
    const char *name;
    classref_t cls;
    //category中所有给类添加的实例方法的列表
    struct method_list_t *instanceMethods;
    //category中所有添加的类方法的列表
    struct method_list_t *classMethods;
    //category实现的所有协议的列表
    struct protocol_list_t *protocols;
    //category中添加的所有属性
    struct property_list_t *instanceProperties;
};
```

从category的定义也可以看出category的能力范围：

* 可为: 可以添加实例方法，类方法，甚至可以实现协议，添加属性
* 不可为: 无法添加实例变量

### Category和方法覆盖

我们都已经知道，category可以为已有的类添加方法，可是当添加方法和类中的方法相同的时候就会将类中原来的方法覆盖，可以说是使用category修改类原有的功能。但是，真如大家所说`覆盖`就无法被调用到了吗？其实`覆盖`这个词在这里表意是非常准确的，category的方法仅仅是覆盖而不是替换了原来的方法，为了避免初学者和笔者之前一样对category的方法覆盖有误解，接下来从2个角度去验证`覆盖`:

> * 阅读objc加载category中的内容过程的源码
> * 调用被分类实现覆盖的类原来实现的方法

objc在运行时会调用`methodizeClass`和`remethodizeClass`为类实现方法就是将类实现阶段将类中和category的方法载入到类的方法列表中。

* **methodizeClass** 

```oc
static void methodizeClass(Class cls)
{
	······ 
    // Attach categories.
    category_list *cats = unattachedCategoriesForClass(cls, true /*realizing*/);
    attachCategories(cls, cats, false /*don't flush caches*/);
    if (cats) free(cats);

}

```

* **remethodizeClass**

```o
static void remethodizeClass(Class cls)
{
	······
    if ((cats = unattachedCategoriesForClass(cls, false/*not realizing*/))) {
        attachCategories(cls, cats, true /*flush caches*/);        
        free(cats);
    }
}

```

* **attachCategories**

methodizeClass 和 remethodizeClass 都是调用attachCategories，attachCategoryMethods做的工作相对比较简单，仅仅是把所有category的实例方法列表拼成了一个大的实例方法列表，然后转交给了attachMethodLists方法: 

```oc
static void 
attachCategories(Class cls, category_list *cats, bool flush_caches)
{
    if (!cats) return;
    if (PrintReplacedMethods) printReplacements(cls, cats);

    bool isMeta = cls->isMetaClass();

    // fixme rearrange to remove these intermediate allocations
    method_list_t **mlists = (method_list_t **)
        malloc(cats->count * sizeof(*mlists));
    property_list_t **proplists = (property_list_t **)
        malloc(cats->count * sizeof(*proplists));
    protocol_list_t **protolists = (protocol_list_t **)
        malloc(cats->count * sizeof(*protolists));
        
    int mcount = 0;
    int propcount = 0;
    int protocount = 0;
    int i = cats->count;
    bool fromBundle = NO;
    while (i--) { 
        auto& entry = cats->list[i];
        if (mlist) {
            mlists[mcount++] = mlist;
            fromBundle |= entry.hi->isBundle();
        }

        property_list_t *proplist = 
            entry.cat->propertiesForMeta(isMeta, entry.hi);
        if (proplist) {
            proplists[propcount++] = proplist;
        }

        protocol_list_t *protolist = entry.cat->protocols;
        if (protolist) {
            protolists[protocount++] = protolist;
        }
    }

    auto rw = cls->data();

    prepareMethodLists(cls, mlists, mcount, NO, fromBundle);
    rw->methods.attachLists(mlists, mcount);
    free(mlists);
    if (flush_caches  &&  mcount > 0) flushCaches(cls);

    rw->properties.attachLists(proplists, propcount);
    free(proplists);

    rw->protocols.attachLists(protolists, protocount);
    free(protolists);
}
```

>  attachCategories 中的 while (i--)   从后完全取可以保证最新的放到最前面

* **attachLists**

  attachLists中主要使用如下两个C函数操作了数组

  > void  memcpy(void *dest, const void *src, size_t n);
  >
  > 从源src所指的内存地址的起始位置开始拷贝n个字节到目标dest所指的内存地址的起始位置中
  >
  > void  memmove( void* dest, const void* src, size_t count );
  >
  > 由src所指内存区域复制count个字节到dest所指内存区域。

```oc
    void attachLists(List* const * addedLists, uint32_t addedCount) {
        if (addedCount == 0) return;

        if (hasArray()) {
            // many lists -> many lists
            uint32_t oldCount = array()->count;
            uint32_t newCount = oldCount + addedCount;
            setArray((array_t *)realloc(array(), array_t::byteSize(newCount)));
            array()->count = newCount;
            memmove(array()->lists + addedCount, array()->lists,
                    oldCount * sizeof(array()->lists[0]));
            memcpy(array()->lists, addedLists,
                   addedCount * sizeof(array()->lists[0]));
        }
        else if (!list  &&  addedCount == 1) {
            // 0 lists -> 1 list
            list = addedLists[0];
        }
        else {
            // 1 list -> many lists
            List* oldList = list;
            uint32_t oldCount = oldList ? 1 : 0;
            uint32_t newCount = oldCount + addedCount;
            setArray((array_t *)malloc(array_t::byteSize(newCount)));
            array()->count = newCount;
            if (oldList) array()->lists[addedCount] = oldList;
            memcpy(array()->lists, addedLists, 
                   addedCount * sizeof(array()->lists[0]));
        }
    }
```

attachLists分三部分:

> 1. many lists -> many lists 
>    * 调用realloc一个新的数组array
>    * 调用memmove将原来的lists放到array的后面
>    * 调用memcpy将addedLists放到array的前面
> 2. 0 lists -> 1 list
>    * `1lsit`没有顺序操作调用list = addedLists[0]直接赋值即可
> 3. 1 list -> many lists
>    * 调用realloc一个新的数组array
>    * array()->lists[addedCount] = oldList 将 原来的一个list放到array的最后的一个位置
>    * 调用memcpy将addedLists放到array的前面

从代码可以看到在处理新加入进来的category的时候没有对原来的lists中的内容进行任何的操作，仅仅做的就是将**旧的移到列表的后面，新的插入到列表的前面**。所以说：

> category的方法不是“替换”原来类已经有的方法，仅仅是“覆盖”，也就是说如果category和原来类都有methodA，那么category附加完成之后，类的方法列表里会有两个methodA

那为什么调用方法的时候总是调用到category的methodA？

> 这样的结果是oc消息机制产生的，这是因为运行时在查找方法的时候是顺着方法列表的顺序查找的，它只要一找到对应名字的方法，就会stop，然后执行查找的imp，它完全不管后面有没有一样名字的方法。



**在类实现后的类列表真的是同时存在所有的实现过的方法吗？** 现在是时候进入在运行时实现的类里面探个究竟了。下面是调试需要的代码:

```oc
@implementation ALin

- (void)printAlin {
    NSLog(@"ALin printAlin");
}
@end

@implementation ALin (Hi)

- (void)printAlin {
    NSLog(@"ALin (Hi) printAlin");
}

@end

@implementation ALin (Bye)

- (void)printAlin {
    NSLog(@"ALin (Hi) printAlin");
}

@end

```

我们在main.m的main函数中调用所有的`printAlin`的实现:

```oc
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        ALin *al = [ALin new];
        Class currentClass = [ALin class];
        if (currentClass) {
            unsigned int methodCount;
            Method *methodList = class_copyMethodList(currentClass, &methodCount);
            IMP imp = NULL;
            SEL sel = NULL;
            for (NSInteger i = 0; i < methodCount; i++) {
                Method method = methodList[i];
                if (sel_registerName("printAlin") == method_getName(method)) {
                    imp = method_getImplementation(method);
                    sel = method_getName(method);
                }
                typedef void (*function)(id,SEL);
                
                if (imp != NULL) {
                   function f = (function)imp;
                    f(al,sel);
                }
            }
            free(methodList);
        }
    return 0;
    }
}
```

执行上面的代码,查看控制的打印结果,可以看出cateorgy`ALin (Hi)` `ALin (Bye)` 和class`ALin`对`printAlin`实现都已经被调用了:

![有帮助的截图]({{ site.url }}/assets/postsImages/category_log_noreplaced.jpg)

> **category添加和原有类同名的方法时，仅仅是覆盖了旧方法，原来的方法还是存在**



### Category的缺点

我们已经知道category可以扩展现有的类，但是category不能完全代替子类，而且有来两个很大的缺点：

1. 覆盖方法的不确定性：

   在Category中无法确定其能够可靠的覆盖某个方法，而这个方法已经在其它的Category中定义过。这个问题在使用Cocoa框架时尤其 突出。当你想覆盖某个框架已经定义好的方法时，该方法已经在其它Category中实现，这样就无法确定哪个定义和实现会被最先使用，带来很大的不确定性。

2. 无法添加实例变量：

   在类中可以自由简便地定义类的实例变量，在category中这定义实例变量。或许有人认为可以通过关联对象的方式来实现添加实例变量。笔者是不同意的，因为关联的对象不属于当前类实现的对象管理的。所以无法添加实例变量是category的一个很大的缺陷。



### the end，baby night



