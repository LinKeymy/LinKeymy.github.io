---
layout: post
title: iOS主线程中使用Runloop实现的功能
subtitle: 
author: JackLin
date: 2017-01-22 20:47:44 +0800
---

Sorry，进门就是一堆代码！

首先我们可以看一下 App 启动后 RunLoop 的状态：
>太长不必细看，下面我会和大家一点一点来

```oc

CFRunLoop {
    current mode = kCFRunLoopDefaultMode
    common modes = {
        UITrackingRunLoopMode
        kCFRunLoopDefaultMode
    }
 
    common mode items = {
 
        // source0 (manual)
        CFRunLoopSource {order =-1, {
            callout = _UIApplicationHandleEventQueue}}
        CFRunLoopSource {order =-1, {
            callout = PurpleEventSignalCallback }}
        CFRunLoopSource {order = 0, {
            callout = FBSSerialQueueRunLoopSourceHandler}}
 
        // source1 (mach port)
        CFRunLoopSource {order = 0,  {port = 17923}}
        CFRunLoopSource {order = 0,  {port = 12039}}
        CFRunLoopSource {order = 0,  {port = 16647}}
        CFRunLoopSource {order =-1, {
            callout = PurpleEventCallback}}
        CFRunLoopSource {order = 0, {port = 2407,
            callout = _ZL20notify_port_callbackP12__CFMachPortPvlS1_}}
        CFRunLoopSource {order = 0, {port = 1c03,
            callout = __IOHIDEventSystemClientAvailabilityCallback}}
        CFRunLoopSource {order = 0, {port = 1b03,
            callout = __IOHIDEventSystemClientQueueCallback}}
        CFRunLoopSource {order = 1, {port = 1903,
            callout = __IOMIGMachPortPortCallback}}
 
        // Ovserver
        CFRunLoopObserver {order = -2147483647, activities = 0x1, // Entry
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
        CFRunLoopObserver {order = 0, activities = 0x20,          // BeforeWaiting
            callout = _UIGestureRecognizerUpdateObserver}
        CFRunLoopObserver {order = 1999000, activities = 0xa0,    // BeforeWaiting | Exit
            callout = _afterCACommitHandler}
        CFRunLoopObserver {order = 2000000, activities = 0xa0,    // BeforeWaiting | Exit
            callout = _ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv}
        CFRunLoopObserver {order = 2147483647, activities = 0xa0, // BeforeWaiting | Exit
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
 
        // Timer
        CFRunLoopTimer {firing = No, interval = 3.1536e+09, tolerance = 0,
            next fire date = 453098071 (-4421.76019 @ 96223387169499),
            callout = _ZN2CAL14timer_callbackEP16__CFRunLoopTimerPv (QuartzCore.framework)}
    },
 
    modes ＝ {
        CFRunLoopMode  {
        	  name =  kCFRunLoopDefaultMode
            sources0 =  { /* same as 'common mode items' */ },
            sources1 =  { /* same as 'common mode items' */ },
            observers = { /* same as 'common mode items' */ },
            timers =    { /* same as 'common mode items' */ },
        },
 
        CFRunLoopMode  {
        	  name = UITrackingRunLoopMode
            sources0 =  { /* same as 'common mode items' */ },
            sources1 =  { /* same as 'common mode items' */ },
            observers = { /* same as 'common mode items' */ },
            timers =    { /* same as 'common mode items' */ },
        },
 
        CFRunLoopMode  {
        	  name = xxxx;
            sources0 = {
                CFRunLoopSource {order = 0, {
                    callout = FBSSerialQueueRunLoopSourceHandler}}
            },
            sources1 = (null),
            observers = {
                CFRunLoopObserver >{activities = 0xa0, order = 2000000,
                    callout = _ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv}
            )},
            timers = (null),
        },
 
        CFRunLoopMode  {
        	  name = //GSEventReceiveRunLoopMode或许不是这个
            sources0 = {
                CFRunLoopSource {order = -1, {
                    callout = PurpleEventSignalCallback}}
            },
            sources1 = {
                CFRunLoopSource {order = -1, {
                    callout = PurpleEventCallback}}
            },
            observers = (null),
            timers = (null),
        },
        
        CFRunLoopMode  {
        	  name = kCFRunLoopCommonModes,
            sources0 = (null),
            sources1 = (null),
            observers = (null),
            timers = (null),
        }
    }
}
```
你会不会好奇到底这么知道这些代码？在下面的方法中添加对应的代码:


```
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
     CFRunLoopRef currentRL = CFRunLoopGetCurrent();
     NSLog(@"timerc:%@",currentRL);
    return YES;
}

```
你会看到很多log出来的内容，我截取一部分出来，如下:


![有帮助的截图]({{ site.url }}/assets/postsImages/mainrunloop01.png)

有看到上面的一部分了吧，有兴趣的伙伴可以自己打印看看，这里就以上面经过整理后的内容来讨论了。
是不是太长了，我们向忽略common mode itmes 和 modes，一起看看:
####  current mode 和 common modes
```oc 
CFRunLoop {
    current mode = kCFRunLoopDefaultMode
    common modes = {
        UITrackingRunLoopMode
        kCFRunLoopDefaultMode
    }
 
    common mode items = { 
    // 先忽略，在common mode items小结具体呢的补充
    },
 
    modes ＝ {
    // 先忽略，在modes小结具体呢的补充
    }
}

```
可以看出app启动后的主线程的Runloop的当前运行模式current mode为kCFRunLoopDefaultMode。设定的common modes有UITrackingRunLoopMode和kCFRunLoopDefaultMode。这也就是为什么将一个NSTimer添加到kCFRunLoopCommonModes的时候，无论是kCFRunLoopDefaultMode还是UITrackingRunLoopMode都能工作的原因了，如果UITrackingRunLoopMode不再里面那么也是无法工作了，不过那么一共有多少种模式呢？一起去看看modes。

### modes

如下是去掉name以外的其他内容:

```oc
modes ＝ {
        CFRunLoopMode  {
        	  name = kCFRunLoopDefaultMode
        },
 
        CFRunLoopMode  {
            name = UITrackingRunLoopMode
        },
 
        CFRunLoopMode  {
        	  name = GSEventReceiveRunLoopMode
        },
        
        CFRunLoopMode  {
            name = kCFRunLoopCommonModes
        }
    }

```
可以看到:
1. kCFRunLoopDefaultMode: App的默认 Mode，通常主线程是在这个 Mode 下运行的。  
2. UITrackingRunLoopMode: 界面跟踪 Mode，用于 ScrollView 追踪触摸滑动，保证界面滑动时不受其他 Mode 影响。   
3: GSEventReceiveRunLoopMode: 接受系统事件的内部 Mode，通常用不到。  
4: kCFRunLoopCommonModes: 这是一个占位的 Mode，没有实际作用。  
>其实App在启动的时候还有另外的一个mode,只是这里看不到而已。   
 
5:UIInitializationRunLoopMode: 在刚启动 App 时第进入的第一个 Mode，启动完成后就不再使用。

那么在这些模式下的Source/Timer/Observer都做了什么呢？系统有如何将收到的网络消息传递给当前app，然后又是如何将文字图片绘制到屏幕，app怎么会识别到手势，和其他事件的呢？这些又和Runloop有什么关系？


### common mode items 和 其他mode里面的items

看看common mode items 

```oc
    common mode items = {
 
        // source0 (manual)
        CFRunLoopSource {order =-1, {
            callout = _UIApplicationHandleEventQueue}}
        CFRunLoopSource {order =-1, {
            callout = PurpleEventSignalCallback }}
        CFRunLoopSource {order = 0, {
            callout = FBSSerialQueueRunLoopSourceHandler}}
 
        // source1 (mach port)
        CFRunLoopSource {order = 0,  {port = 17923}}
        CFRunLoopSource {order = 0,  {port = 12039}}
        CFRunLoopSource {order = 0,  {port = 16647}}
        CFRunLoopSource {order =-1, {
            callout = PurpleEventCallback}}
        CFRunLoopSource {order = 0, {port = 2407,
            callout = _ZL20notify_port_callbackP12__CFMachPortPvlS1_}}
        CFRunLoopSource {order = 0, {port = 1c03,
            callout = __IOHIDEventSystemClientAvailabilityCallback}}
        CFRunLoopSource {order = 0, {port = 1b03,
            callout = __IOHIDEventSystemClientQueueCallback}}
        CFRunLoopSource {order = 1, {port = 1903,
            callout = __IOMIGMachPortPortCallback}}
 
        // Ovserver
        CFRunLoopObserver {order = -2147483647, activities = 0x1, // Entry
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
        CFRunLoopObserver {order = 0, activities = 0x20,          // BeforeWaiting
            callout = _UIGestureRecognizerUpdateObserver}
        CFRunLoopObserver {order = 1999000, activities = 0xa0,    // BeforeWaiting | Exit
            callout = _afterCACommitHandler}
        CFRunLoopObserver {order = 2000000, activities = 0xa0,    // BeforeWaiting | Exit
            callout = _ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv}
        CFRunLoopObserver {order = 2147483647, activities = 0xa0, // BeforeWaiting | Exit
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
 
        // Timer
        CFRunLoopTimer {firing = No, interval = 3.1536e+09, tolerance = 0,
            next fire date = 453098071 (-4421.76019 @ 96223387169499),
            callout = _ZN2CAL14timer_callbackEP16__CFRunLoopTimerPv (QuartzCore.framework)}
    },
```
无论是source/observer/tiemr对应的callout，这些就是在Runloop运行的时候要执行的回调。当 RunLoop 进行回调时，一般都是通过一个很长的函数调用出去 (call out), 当你在你的代码中下断点调试时，通常能在调用栈上看到这些函数。下面是这几个函数的整理版本，如果你在调用栈中看到这些长函数名，在这里查找一下就能定位到具体的调用地点了

```oc 

{
    /// 1. 通知Observers，即将进入RunLoop
    /// 此处有Observer会创建AutoreleasePool: _objc_autoreleasePoolPush();
    __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopEntry);
    do {
 
        /// 2. 通知 Observers: 即将触发 Timer 回调。
        __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopBeforeTimers);
        /// 3. 通知 Observers: 即将触发 Source (非基于port的,Source0) 回调。
        __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopBeforeSources);
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(block);
 
        /// 4. 触发 Source0 (非基于port的) 回调。
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(source0);
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(block);
 
        /// 6. 通知Observers，即将进入休眠
        /// 此处有Observer释放并新建AutoreleasePool: _objc_autoreleasePoolPop(); _objc_autoreleasePoolPush();
        __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopBeforeWaiting);
 
        /// 7. sleep to wait msg.
        mach_msg() -> mach_msg_trap();
 
        /// 8. 通知Observers，线程被唤醒
        __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopAfterWaiting);
 
        /// 9. 如果是被Timer唤醒的，回调Timer
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(timer);
 
        /// 9. 如果是被dispatch唤醒的，执行所有调用 dispatch_async 等方法放入main queue 的 block
        __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(dispatched_block);
 
        /// 9. 如果如果Runloop是被 Source1 (基于port的) 的事件唤醒了，处理这个事件
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__(source1);
 
 
    } while (...);
 
    /// 10. 通知Observers，即将退出RunLoop
    /// 此处有Observer释放AutoreleasePool: _objc_autoreleasePoolPop();
    __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopExit);
}

```
上面的已经有了部分的注释，下面分开了具体看看

#### Autoreleasepool


在common modte itmes中有如下连个CFRunLoopObserver

```oc

CFRunLoopObserver {order = -2147483647, activities = 0x1, // Entry
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
CFRunLoopObserver {order = 2147483647, activities = 0xa0, // BeforeWaiting | Exit
            callout = _wrapRunLoopWithAutoreleasePoolHandler}
```
在Runloop的逻辑代码中先将和 Entry ，BeforeWaiting | Exit的其他内容去掉如下

```oc

{
    /// 1. 通知Observers，即将进入RunLoop
    /// 此处有Observer会创建AutoreleasePool: _objc_autoreleasePoolPush();
    __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopEntry);
    do {
 
        /// 6. 通知Observers，即将进入休眠
        /// 此处有Observer释放并新建AutoreleasePool: _objc_autoreleasePoolPop();

    } while (...);
 
    /// 10. 通知Observers，即将退出RunLoop
    /// 此处有Observer释放AutoreleasePool: _objc_autoreleasePoolPop();
    __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(kCFRunLoopExit);
}
```



可见，App启动后，苹果在主线程 RunLoop 里注册了两个 Observer，其回调都是 _wrapRunLoopWithAutoreleasePoolHandler()。

第一个 Observer 监视的事件是 Entry(即将进入Loop)，其回调内会调用 _objc_autoreleasePoolPush() 创建自动释放池。其 order 是-2147483647，优先级最高，保证创建释放池发生在其他所有回调之前。

第二个 Observer 监视了两个事件： BeforeWaiting(准备进入休眠) 时调用_objc_autoreleasePoolPop() 和 _objc_autoreleasePoolPush() 释放旧的池并创建新池；Exit(即将退出Loop) 时调用 _objc_autoreleasePoolPop() 来释放自动释放池。这个 Observer 的 order 是 2147483647，优先级最低，保证其释放池子发生在其他所有回调之后。

##### 所以在主线程执行的代码，通常是写在诸如事件回调、Timer回调内的。这些回调会被 RunLoop 创建好的 AutoreleasePool 环绕着，所以不会出现内存泄漏，开发者也不必显示创建Pool了。


#### 事件响应

```oc
 // source0 非端口
 CFRunLoopSource {order =-1, {
            callout = _UIApplicationHandleEventQueue}}
 // source1 基于端口port
 CFRunLoopSource {order = 0, {port = 1b03,
            callout = __IOHIDEventSystemClientQueueCallback}}
```

苹果注册了一个 Source1 (基于 mach port 的,这里port = 1b03) 用来接收系统事件，其回调函数为 __IOHIDEventSystemClientQueueCallback()。

当一个硬件事件(触摸/锁屏/摇晃等)发生后，首先由 IOKit.framework 生成一个 IOHIDEvent 事件并由 SpringBoard 接收。SpringBoard 只接收按键(锁屏/静音等)，触摸，加速，接近传感器等几种 Event，随后用 mach port 转发给需要的App进程。随后苹果注册的那个 Source1 就会触发回调，并调用 _UIApplicationHandleEventQueue() 进行应用内部的分发。

_UIApplicationHandleEventQueue() 会把 IOHIDEvent 处理并包装成 UIEvent 进行处理或分发，事件加入到一个由UIApplication管理的事件队列中，其中包括识别 UIGesture/处理屏幕旋转/等，随后便是hitTest与pointInside，事件会按照UIApplication -> UIWindow -> SuperView -> SubView的顺序不断的检测。通常事件比如 UIButton 点击、touchesBegin/Move/End/Cancel 事件都是在这个回调中完成的。关于事件传递和响应者链条这里就不讨论了。感兴趣可以到简书里面看看这一篇[文章](https://www.jianshu.com/p/77a1b6e5194d)；

##### 这里突然想到第一响应者的问题，区分一下上面提到的响应者链条的第一个响应者。


第一响应者 (The First Responder)，什么是第一响应者？简单的讲，第一响应者是一个UIWindow对象接收到一个事件后，第一个来响应的该事件的对象。注意：这个第一响应者与之前讨论的触摸检测到的第一个响应的UIView并不是一个概念。第一响应者一般情况下用于处理非触摸事件（手机摇晃、耳机线控的远程空间）或非本窗口的触摸事件（键盘触摸事件），通俗点讲其实就是管别人闲事的响应者。在IOS中，当然管闲事并不是所有控件都愿意的，这么说好像并不是很好理解，或着是站在编程人员的角度来看待这个问题，程序员负责告诉系统哪个对象可以成为第一响应者(canBecomeFirstResponder)，如果方法canBecomeFirstResponder返回YES，这个响应者对象才有资格称为第一响应者。有资格并不代表一定可以成为第一响应者，就好像符合要求并不一定能够应聘成功一样，所以还差一个聘用环节，那就是becomeFirstResponder正式成为第一响应者。

　　请原谅我的这些可能不太正常的想法，个人感觉上面的过程又有点像招聘流程，简历筛选就是canBecomeFirstResponder，becomeFirstResponder就是正式成为公司的员工。那么既然公司由聘用，那么对应的就有辞退咯！对应的方法就是canResignFirstResponder，这个表示第一响应者是否可以被辞退，有些牛逼到逆天的员工并不是说辞退就辞退的，争取有一天可以成为这个逆天员工，好吧，我又扯远了。还有一个方法就是resignFirstResponder，正式辞退该员工。

　　值得注意的是，一个UIWindow对象在某一时刻只能有一个响应者对象可以成为第一响应者。我们可以通过isFirstResponder来判断某一个对象是否为第一响应者。 


#### 手势识别

当上面的 _UIApplicationHandleEventQueue() 识别了一个手势时，其首先会调用 Cancel 将当前的 touchesBegin/Move/End 系列回调打断。随后将对应的 UIGestureRecognizer 标记为待处理。

苹果注册了一个 Observer 监测 BeforeWaiting (Loop即将进入休眠) 事件，这个Observer的回调函数是 _UIGestureRecognizerUpdateObserver()，其内部会获取所有刚被标记为待处理的 UIGestureRecognizer，并执行UIGestureRecognizer的回调。

```oc 
   CFRunLoopObserver {order = 0, activities = 0x20,          // BeforeWaiting
            callout = _UIGestureRecognizerUpdateObserver}

```
当有 UIGestureRecognizer 的变化(创建/销毁/状态改变)时，这个回调都会进行相应处理。

#### 界面刷新
当在操作 UI 时，比如改变了 Frame、更新了 UIView/CALayer 的层次时，或者手动调用了 UIView/CALayer 的 setNeedsLayout/setNeedsDisplay方法后，这个 UIView/CALayer 就被标记为待处理，并被提交到一个全局的容器去。（很多事件都是这样，先标记好是待处理，然后在进入休眠或者推出前去处理他们）

```oc 
CFRunLoopObserver {order = 2000000, activities = 0xa0,    // BeforeWaiting | Exit
            callout = _ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv}

```
如上，苹果向commonmode下的Runloop注册了一个 Observer 监听 BeforeWaiting(即将进入休眠) 和 Exit (即将退出Loop) 事件，回调去执行一个很长的函数：
_ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv()。这个函数里会遍历所有待处理的 UIView/CAlayer 以执行实际的绘制和调整，并更新 UI 界面。

#### GCD
GCD 提供的某些接口也用到了 RunLoop， 例如 dispatch_async()。当调用 dispatch_async(dispatch_get_main_queue(), block) 时，libDispatch 会向主线程的 RunLoop 发送消息，RunLoop会被唤醒，并从消息中取得这个 block，并在回调 __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__() 里执行这个 block。但这个逻辑仅限于 dispatch 到主线程，dispatch 到其他线程仍然是由 libDispatch 处理的。

```oc
     /// 9. 如果是被dispatch唤醒的，执行所有调用 dispatch_async 等方法放入main queue 的 block
      __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(dispatched_block);
```

#### 定时器

在[NSTimer的官方文档](https://developer.apple.com/documentation/foundation/timer#//apple_ref/occ/cl/NSTimer)上，苹果是这么说的：
>A timer is not a real-time mechanism; it fires only when one of the run loop modes to which the timer has been added is running and able to check if the timer’s firing time has passed.

意思就是说，NSTimer并不是一种实时机制，它只会在下面条件满足的情况下才会启动:
>1.NSTimer被添加到的RunLoop模式正在运行。  
2.NSTimer设定的启动时间还没有过去。

所以在使用timer的时候一定要注意者两点

NSTimer 其实就是 CFRunLoopTimerRef，他们之间是 toll-free bridged 的。一个 NSTimer 注册到 RunLoop 后，RunLoop 会为其重复的时间点注册好事件。例如 10:00, 10:10, 10:20 这几个时间点。RunLoop为了节省资源，并不会在非常准确的时间点回调这个Timer。Timer 有个属性叫做 Tolerance (宽容度)，标示了当时间点到后，容许有多少最大误差。`设定时间 <= NSTimer的启动时间 <= 设定时间 + tolerance`

在[NSTimer的官方文档](https://developer.apple.com/documentation/foundation/timer#//apple_ref/occ/cl/NSTimer)上，苹果是这么说的：
>Setting a tolerance for a timer allows it to fire later than the scheduled fire date, improving the ability of the system to optimize for increased power savings and responsiveness.

意思就是，设定容差可以起到省电和优化系统响应性的作用。果不设定的话，默认为0。哪怕为0，系统依然有权利去设置一个很小的容差。

如果某个时间点被错过了，例如执行了一个很长的任务，则那个时间点的回调也会跳过去，不会延后执行。就比如等公交，如果 10:10 时我忙着玩手机错过了那个点的公交，那我只能等 10:20 这一趟了。所以NSTimer是否精确，很大程度上取决于线程当前的空闲情况。即使当前线程并不阻塞，系统依然可能会在NSTimer上加上很小的的容差。

`如何正确的终止timer？`

NSTimer提供了一个invalidate方法，用于终止NSTimer。但是，这里涉及到一个多线程的问题。假设，我在A线程启动NSTimer，在B线程调用invalidate方法来终止NSTimer，那么，NSTimer是否会终止呢。

我们来试验一下：

```oc
dispatch_async(dispatch_get_main_queue(), ^{
        self.timer = [NSTimer scheduledTimerWithTimeInterval:0.5f
                                                      target:self
                                                    selector:@selector(test)
                                                    userInfo:nil
                                                     repeats:YES];
    });

dispatch_async(dispatch_get_global_queue(0, 0), ^{
        [self.timer invalidate];
    });
```
结果是并不会停止。必须哪个线程调用，哪个线程终止
>
You should always call the invalidate method from the same thread on which the timer was installed.

因此在使用Timer的时候要注意多线程带来的问题。

CADisplayLink 是一个和屏幕刷新率一致的定时器（但实际实现原理更复杂，和 NSTimer 并不一样，其内部实际是操作了一个 Source）。

小结一下使用timer要注意的点

* Timer的启动

> 1.NSTimer被添加到的RunLoop模式正在运行。  
  2.NSTimer设定的启动时间还没有过去。
 
 * Timer的精确度
 
> 1.当前线程的空闲度会影响  
  2.系统会设置一个容差度
  
 * Timer的终止
 
> 必须哪个线程调用，哪个线程终止 

 

#### PerformSelecter

当调用 NSObject 的 performSelecter:afterDelay: 后，实际上其内部会创建一个 Timer 并添加到当前线程的 RunLoop 中。所以如果当前线程没有 RunLoop，则这个方法会失效。

当调用 performSelector:onThread: 时，实际上其会创建一个 Timer 加到对应的线程去，同样的，如果对应线程没有 RunLoop 该方法也会失效。

#### 关于网络请求（待续）