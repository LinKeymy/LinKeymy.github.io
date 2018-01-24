---
layout: post
title: const,static,extern
subtitle: 
author: JackLin
date: 2018-01-24 23:09:14 +0800
---

### const与宏

* `执行时刻`:宏是预编译（编译之前先做替换），const是编译阶段。
* `编译检查`:宏定义中不做检查，不会报编译错误，在使用处替换后，才有可能检查出错误，const会编译检查，会报编译错误。
* `宏的好处`:宏能定义一些函数，方法。 const不能。
* `宏的坏处`:使用大量宏，每次都需要重新替换，容易造成预编译时间过长，从而导致编译时间过长。

>Apple推荐使用const而不是宏，而且在swift中Apple已经将宏去掉了。

使用宏函数是，有很多的注意点，这里不过多去说了。
关于宏的使用细节和注意点推荐大家前往传送们[宏定义的黑魔法 - 宏菜鸟起飞手册](https://onevcat.com/2014/01/black-magic-in-macro/)，非常推荐大家去看看王巍大神的blog。
	
### const作用
* const仅仅用来修饰右边的变量,被const修饰的变量是只读的。

#### const:修饰基本变量

```
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // 这两种写法是一样的，const只修饰右边的基本变量b
        // const int b = 20; // b:只读变量
        //  int const c = 20; // b:只读变量c
        //  / 不允许修改值
        //  b = 1;   //编译报错
        //  c = 1;   //编译报错
        int a = 100;
        int b = 1000;
        
        int * const p;  // p:只读  *p:变量 // 指向的地址无法更改，但是指向的地址对应的变量可以更改
        *p = a;
        // p = &a; //编译报错
        
        int const * p1; // p1:变量 *p1:只读 // 指向的地址可以更改
        p1 = &a;
        p1 = &b;
        // *p1 = 100000;  //编译报错
        const int * p2; // p2:变量 *p2:只读
        p2 = &a;
        p2 = &b;
        // *p2 = 100000;  //编译报错
        const int * const p3; // p3:只读 *p3:只读
        // *p3 = a;     //编译报错
        //  p3 = &a;      //编译报错
        int const * const p4; // p4:只读 *p4:只读
        // *p4 = a;    //编译报错
        // p4 = &a;      //编译报错
    }
    return 0;
}

```

#### const:修饰和指针相关的变量

```oc
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        int * const p;  // p:只读  *p:变量 // 指向的地址无法更改，但是指向的地址对应的变量可以更改
        *p = a;
        // p = &a; //编译报错
        
        int const * p1; // p1:变量 *p1:只读 // 指向的地址可以更改
        p1 = &a;
        p1 = &b;
        // *p1 = 100000;  //编译报错
        const int * p2; // p2:变量 *p2:只读
        p2 = &a;
        p2 = &b;
        // *p2 = 100000;  //编译报错
        const int * const p3; // p3:只读 *p3:只读
        // *p3 = a;     //编译报错
        // p3 = &a;      //编译报错
        int const * const p4; // p4:只读 *p4:只读
        // *p4 = a;    //编译报错
        // p4 = &a;      //编译报错
    }
    return 0;
}

```


### const具体的使用

#### 替代宏修饰全局的变量 

如在UIKit的Window.h的头文件中，Apple就大量使用了const

```oc
UIKIT_EXTERN NSString *const UIKeyboardFrameBeginUserInfoKey        NS_AVAILABLE_IOS(3_2) __TVOS_PROHIBITED; // NSValue of CGRect
UIKIT_EXTERN NSString *const UIKeyboardFrameEndUserInfoKey          NS_AVAILABLE_IOS(3_2) __TVOS_PROHIBITED; // NSValue of CGRect
UIKIT_EXTERN NSString *const UIKeyboardAnimationDurationUserInfoKey NS_AVAILABLE_IOS(3_0) __TVOS_PROHIBITED; // NSNumber of double
UIKIT_EXTERN NSString *const UIKeyboardAnimationCurveUserInfoKey    NS_AVAILABLE_IOS(3_0) __TVOS_PROHIBITED; // NSNumber of NSUInteger (UIViewAnimationCurve)
UIKIT_EXTERN NSString *const UIKeyboardIsLocalUserInfoKey           NS_AVAILABLE_IOS(9_0) __TVOS_PROHIBITED; // NSNumber of BOOL
```


#### 修饰函数中传入的参数

* 1.参数是地址，里面只能通过地址读取值,不能通过地址修改值
* 2.这个方法的参数是地址，里面不能修改参数的地址。

不过这样的使用在iOS中较少用到
```

void example1(int const *a){
    // const放*前面约束参数，表示*a只读
    // 只能修改地址a,不能通过a修改访问的内存空间
    int b = 100;
    a = &b;  //修改的参数，不是改变外部变量保存的地址
    //    *a = 1000; // 编译不通过
}

//函数的参数是地址，里面不能修改参数的地址。但是可以修改对应地址保存的值
void example2(int * const a){
    int b = 10000;
    //    a = &b; // 编译不通过
    *a = 1000;
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        int a = 10000000;
        example1(&a);
        NSLog(@"%d",a);
        example2(&a);
        NSLog(@"%d",a);
    }
    return 0;
}

@end
```

### static和extern

##### static修饰局部变量   
* 被static修饰局部变量,延长生命周期,跟整个应用程序有关
* 被static修饰局部变量,程序一运行就会给static修饰变量分配内存,只会分配一次内存,
		
```oc

@implementation Farter

- (void)callSon {
    NSLog(@"call_son");
}

- (void)staticTest {
    static int b = 100;
    NSLog(@"%p",&b);
}

@end


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [[Farter alloc] init];
        [f staticTest];
        [f staticTest];
        [f staticTest];
    }
    return 0;
}

```
上面的打印结果如下:

![有帮助的截图]({{ site.url }}/assets/postsImages/static_local.png)

>可以看到执行多次[f staticTest]，b的地址没有发生改变。
		
##### static修饰全局变量,相当于文件私有		
		
static,修饰全局变量,被static修饰全局变量,作用域会修改,只能在当前文件下使用

下面定义了一个全局的 staticVar 然然后在staticTest2和staticTest中都访问了它

```oc 

@implementation Farter

static int staticVar;

-(void)staticTest2 {
    NSLog(@"staticTest2");
    staticVar = 100;
    NSLog(@"%p",&staticVar);
}

- (void)staticTest {
    staticVar = 100;
    NSLog(@"%p",&staticVar);
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [[Farter alloc] init];
        [f staticTest];
        [f staticTest];
        [f staticTest];
        [f staticTest2];
    }
    return 0;
}

```
![有帮助的截图]({{ site.url }}/assets/postsImages/static_global.png)

> 很合理，staticVar被整个文件访问到了，而且地址也不会改变，毕竟就声明一次。


但是我怎么可以让在`Farter.m`文件中的变量staticVar可以直接在`main.m`文件中被访问呢？，extern可以来帮忙。

#### extern声明一个全局变量
##### 要点
* extern:声明外部全局变量,extern只能用于声明,不能用于定义
* extern工作原理:先会去当前文件下查找有没有对应全局变量,如果没有,才回去其他文件查找
* static修饰的变量，extern声明无效，编译报错	

		
```oc
extern int staticVar;
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Farter *f = [[Farter alloc] init];
        [f staticTest];
        [f staticTest];
        [f staticTest];
        [f staticTest2];
        staticVar = 1000;
    }
    return 0;
}

```		
如上，如果只是在代码的`main.m`文件中	 extern int staticVar，而没有去掉`Farter.m`中static int staticVar的static	,那么编译会不通过，表示	在`main.m`中staticVar没有被定义。也就是证明了，static修饰的变量，extern声明无效。
		
![有帮助的截图]({{ site.url }}/assets/postsImages/static_extern_error.png)	


去掉`Farter.m`中static int staticVar的static,那么编译通过,打印结果和单独使用static修饰全局变量类似，变量的地址不会被改变

![有帮助的截图]({{ site.url }}/assets/postsImages/extern_right.png)
	

### static、extern和const结合

#### static const 结合

* 声明在`一个文件中`只读的静态变量，static与const作用（可以简单认为就需要一个文件私有的静态常量）

```oc
static  NSString * const name = @"onevlin";
static int const age = 10;

```

#### extern与const结合

* 如果需要一个在`多个文件中`都能访问的全局常量，可以使用extern与const组合

如下，在`Farter.h`文件中声明一个name，然后在`Farter.m`中定义，随后只要在需要用的地方导入`Farter.h `既可

```oc
Farter.h

extern NSString * const name;


Farter.m

NSString * const name = @"onevlin";

#import "Farter.h"

extern int staticVar;
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSLog(@"name:%@",name);
    }
    return 0;
}

```
上面编译通过，打印正常输出onevlin



Apple在UIKit等框架中大量使用了类似方法定义了大量的全局的常量字符串的key,比如UIKit的UIApplicaton.h

![有帮助的截图]({{ site.url }}/assets/postsImages/UIApplication_header.png)


### 总结

const 用于限制放在修饰的变量被修改

static和extern用于限制变量的访问权限，static修饰的变量为文件私有，而extern为全局的


> 可能会有一些不对的地方，欢迎指出