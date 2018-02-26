---
layout: post
title: NSOperationQueue
subtitle: 
author: JackLin
date: 2018-02-25 23:34:05 +0800
---

### 前言

上一篇文章的介绍了GCD：Grand Central Dispatch，它是一组相对低层的C语言API。尽管GCD对线程管理进行了封装，如果我们要管理队列中的任务（例如：查看任务状态、取消任务、控制任务之间的执行顺序等）仍然不是很方便。为此，iOS基于GCD对多线程任务进行了进一步封装，提供了一个面向对象方式的多任务执行机制，叫做：**operation queue**。operation queue和Grand Central Dispatch有很多相似的概念，它们都是基于任务和队列的概念进行编程。这很正常，毕竟operation queue是基于GCD的封装。这篇文章我们一起看看如何使用operation queue。

### NSOperation和NSOperationQueue

和GCD一样，我们可以把operation queue也认为是一个队列，这个队列在iOS中用NSOperationQueue表示，添加到operation queue中的任务除了会被并行执行外，还有它们自己的特点：

- 它们并不遵从FIFO的原则；
- 它们不再是简单的closure，而是被封装成了一个NSOperation类；

但是**NSOperation是一个抽象类**，我们**不能直接生成一个NSOperation对象**，iOS有两个基于NSOperation实现了两个具象类：

- NSBlockOperation - 我们可以把它理解为是一个closure的封装；
- NSInvocationOperation - 在Objective-C中通过selector指定要调用的任务；

因此使用NSOperation子类的方式有3种：

1. 使用NSInvocationOperation
2. 使用NSBlockOperation
3. 自定义子类继承NSOperation

### 使用NSInvocationOperation

先创建`NSInvocationOperation`对象

```objective-c
NSInvocationOperation *invocationOperation =
        [[NSInvocationOperation alloc]
                initWithTarget:self
                      selector:@selector(task01)
                        object:nil];
```

想让`invocationOperation`中的selector执行有两种方式：

**第一是调用`-(void)start`方法：**

```objective-c
[invocationOperation start];
```

创建工程在viewDidLoad中添加如下代码：

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];
    NSInvocationOperation *invocationOperation =
    [[NSInvocationOperation alloc]
            initWithTarget:self
                  selector:@selector(task01)
                    object:nil];
    [invocationOperation start];
}

- (void)task01 {
    NSLog(@"\ntask01: %@",[NSThread currentThread].description);
}
```

查看运行结果：

![有帮助的截图]({{ site.url }}/assets/postsImages/operation-queue-start.png)

*number = 1,看来调用了start方法后并不会开一条新线程去执行操作，而是在当前线程同步执行操作*

**将`invocationOperation`添加到NSOperationQueue中**

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];
    NSInvocationOperation *invocationOperation =
    [[NSInvocationOperation alloc]
            initWithTarget:self
                  selector:@selector(task01)
                    object:nil];
    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue addOperation:invocationOperation];
}

- (void)task01 {
    NSLog(@"\ntask01: %@",[NSThread currentThread].description);
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operation-queue-addInvocation.png)

*number = 3,NSInvocationOperation放到一个NSOperationQueue中，会异步执行操作*

其实NSOperation和NSOperationQueue的大概有如下步骤:

1. 将需要执行的操作封装到一个NSOperation对象中
2. 将NSOperation对象添加到NSOperationQueue中
3. 系统**自动**将NSOperationQueue中的NSOperation取出来
4. 取出的NSOperation封装的操作放到一条**新线程**中执行

> 值得一提的是，由于使用NSInvocationOperation可能带来的类型安全以及ARC安全的问题，需要注意的是在Swift中Apple从iOS 8.1开始去掉了这个类。大家可以在[iOS 8.1 API Diffs](https://developer.apple.com/library/ios/releasenotes/General/iOS81APIDiffs/modules/Foundation.html)看到这个变更。因此，推荐无论Objective-C还是Swift我们多习惯使用NSBlockOperation就好了。

### 使用NSBlockOperation

创建NSBlockOperation对象,并添加多个block任务到blockOperation中：

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSBlockOperation *blockOperation =
            [NSBlockOperation blockOperationWithBlock:^{
                printf("blockOperation ==== task01 : %s\n",
                        [NSThread currentThread].description.UTF8String);
            }];

    [blockOperation addExecutionBlock:^{
        printf("blockOperation ==== task02 : %s\n",
                [NSThread currentThread].description.UTF8String);
    }];
    [blockOperation addExecutionBlock:^{
        printf("blockOperation ==== task03 : %s\n",
                [NSThread currentThread].description.UTF8String);
    }];
    blockOperation.completionBlock = ^ {
        printf("blockOperation = completionBlock");
    };
    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue addOperation:blockOperation];
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operation-queue-blockOperation.png)



可以看到所有被添加到queue的任务都会被并行执。而且NSBlockOperation所有的任务执行完成后被调用设置的completionBlock。这样可以实现多个并发任务都执行完成后再执行其他任务的需求。

除了显性使用`NSBlockOperation`对象，我们也可以使用addOperationWithBlock直接向operation queue中添加一个closure，它会被自动转换成NSBlockOperation：

```objective-c
    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue addOperationWithBlock:^{
        printf("addOperationWithBlock ==== task01 : %s\n",
                [NSThread currentThread].description.UTF8String);
    }];
    [queue addOperationWithBlock:^{
        printf("addOperationWithBlock ==== task02 : %s\n",
                [NSThread currentThread].description.UTF8String);
    }];
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operation-queue-addOperationWithBlock.png)



**继续往下之前，先小结一下上面的内容：**

1. NSOperation可以调用start方法来执行任务，但默认是同步执行的
2. 将NSOperation添加到NSOperationQueue中，系统会自动异步执行NSOperation中的操作
3. 将NSOperation添加操作到NSOperationQueue中有两种方法：
   * \- (void)addOperation:(NSOperation*)operation;
   * \- (void)addOperationWithBlock:(void (^)(void))block;

### 最大并发数: maxConcurrentOperationCount

并发数指的是同时执行的任务数，比如，同时开32个线程执行2个任务，并发数就是2。最大并发，从名字就很容易理解，限定queue中的任务能同时执行的最大任务数。如果设置maxConcurrentOperationCount的值为1.那么queue的任务只能一个接一个执行，没有并发，但是和GCD中的串行队列不同，这里的每个任务执行的线程是不确定的，它们可以不在同一条线程中被执行：

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue setMaxConcurrentOperationCount:1];
    for (int i = 0; i < 10; ++i) {
        [queue addOperationWithBlock:^{
            printf("addOperationWithBlock ==== task0%i : %s\n",
                    i,[NSThread currentThread].description.UTF8String);
        }];
    }
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/queue_setMaxConcurrentOperationCount.png)



### 设置操作之间的关联性(操作依赖)

在一开始就提到过，operation queue提供了更细粒度的任务控制，我们可以设置一个队列中不同任务的关联性也就是操作依赖，让它们先后执行。比如一定要让操作A执行完后，才能执行操作B，可以这么写：

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    
    NSBlockOperation *blockOperationA =
            [NSBlockOperation blockOperationWithBlock:^{
                [NSThread sleepForTimeInterval:3];
                printf("=========  blockOperationA =========\n");
            }];
    
    NSBlockOperation *blockOperationB =
            [NSBlockOperation blockOperationWithBlock:^{
                printf("=========  blockOperationB =========\n");
            }];
    // 操作依赖
    [blockOperationB addDependency:blockOperationA];
    
    [queue addOperation:blockOperationA];
    [queue addOperation:blockOperationB];
}
```

`[blockOperationB addDependency:blockOperationA]`设置操作B依赖于操作A，那么操作B必须等操作A执行完成后才能被执行。

![有帮助的截图]({{ site.url }}/assets/postsImages/operation_addDependency_a_b.png)

如果不添加依赖则无法保证queue中operation的执行顺序，比如将上面的操作依赖注释重新运行：

```objective-c
// [blockOperationB addDependency:blockOperationA]
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operation_dependecy_a_b.png)

**可以在不同queue的NSOperation之间创建依赖关系**

修改上面的代码，添加多一队列和一个blockOperationC,并且设置operation间的操作依赖。具体代码如下：

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSOperationQueue *queue0 = [[NSOperationQueue alloc] init];
    NSOperationQueue *queue1 = [[NSOperationQueue alloc] init];
    
    NSBlockOperation *blockOperationA =
            [NSBlockOperation blockOperationWithBlock:^{
                [NSThread sleepForTimeInterval:3];
                printf("=========  blockOperationA =========\n");
            }];
    NSBlockOperation *blockOperationB =
            [NSBlockOperation blockOperationWithBlock:^{
                printf("=========  blockOperationB =========\n");
            }];
    NSBlockOperation *blockOperationC =
            [NSBlockOperation blockOperationWithBlock:^{
                printf("=========  blockOperationC =========\n");
            }];
    
    [blockOperationB addDependency:blockOperationA];
    [blockOperationC addDependency:blockOperationB];
	
    [queue1 addOperation:blockOperationC];
    [queue0 addOperation:blockOperationA];
    [queue0 addOperation:blockOperationB];
   
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operaton_dependency_diffurent_queue.png)

### 队列的取消、暂停、恢复

除了设置依赖关系，我们可以随时取消一个operation queue中所有的任务，但是取消的结果根据任务的状态会有所不同：

- 所有已经完成的任务，取消操作不会有任何结果；
- 如果一个任务被取消，所有和它关联的任务也会被取消；
- 任务被取消之后，completionBlock仍旧会被执行；

#### **队列的取消**

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue setMaxConcurrentOperationCount:3];  // 设置最大并发为3
    for (int i = 0; i < 150; ++i) {
        [queue addOperationWithBlock:^{
            [NSThread sleepForTimeInterval:1]; // sleep 1 秒，模拟耗时
            printf("addOperationWithBlock ==== task0%i : %s\n",
                    i,[NSThread currentThread].description.UTF8String);
        }];
    }

    dispatch_queue_t global_queue =
            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    dispatch_time_t when =
            dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC));
	// 3 秒后取消所有操作
    dispatch_after(when, global_queue, ^{
        [queue cancelAllOperations];
    });
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/queue_cancelAllOperations.png)

> 也可以调用NSOperation的- (void)cancel方法取消单个操作

#### **暂停和恢复队列**

`-(void)setSuspended:(BOOL)bool`    YES代表暂停队列，NO代表恢复队列

`-(BOOL)isSuspended`查看队列的挂起状态

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];

    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue setMaxConcurrentOperationCount:2];  // 设置最大并发为3
    
    for (int i = 0; i < 5; ++i) {
        [queue addOperationWithBlock:^{
            printf("addOperationWithBlock ==== task0%i : %s\n",
                    i,[NSThread currentThread].description.UTF8String);
            [NSThread sleepForTimeInterval:3]; // sleep 1 秒，模拟耗时
        }];
    }

    dispatch_queue_t global_queue =
            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    dispatch_time_t suspendedTime =
            dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC));
    dispatch_time_t unSuspendedTime =
    dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10.0 * NSEC_PER_SEC));
    // 2 秒后挂起
    dispatch_after(suspendedTime, global_queue, ^{
        [queue setSuspended:YES];
        NSLog(@"++++++++++++++++  isSuspended: %i",queue.isSuspended);
        // 2 秒后取消挂起
        dispatch_after(unSuspendedTime, global_queue, ^{
            [queue setSuspended:NO];
            NSLog(@"++++++++++++++++  isSuspended: %i",queue.isSuspended);
        });
    });
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operation_setSuspended.png)

可以看到任务线程在第一轮队列并发的时候执行了2个任务，然后睡3秒，线程睡的期间queue挂起，挂起期间队列queue中的任务都暂停执行。在等8秒后队列queue取消挂起，然后又开始将队列中的任务异步派发到线程中执行。

### 自定义NSOperation

如果 block operation 和 invocation operation 对象不符合应用的需求，可以直接继承 NSOperation，并添加任何你想要的行为。NSOperation类提供通用的子类继承点，而且实现了许多重要的基础设施来处理依赖和 KVO 通知。继承所需的工作量主要取决于你要实现非并发还是并发的operation。

#### **自定义非并发的operation**	

定义非并发 operation 要简单许多，只需要执行主任务，并正确地响应取消事件;NSOperation 处理了其它所有事情。

> 需要说明的是这里说的非并发指的是在使用start方法的情况下的。如果将operation添加到queue中都是异步执行的。

自定义一个operation：`NonConcurrentOperation`,重写`main`方法

```objective-c
@implementation NonConcurrentOperation
- (instancetype)initWithName:(NSString *)name {
    if (self = [super init]) {
        self.name = name;
    }
    return self;
}
    
- (void)main {
    @try { 
        // 很多时候这个并不是在mainqueue中的operation。不过在主线程中start的除外。
        @autoreleasepool {
            NSLog(@"==== 执行自定义非并发NSOperation ====");
            if (self.isCancelled) {
                printf("isCancelled: %s\n",self.name.UTF8String);
                return;
            };
            printf("name: %s , thread: %s\n",
                    self.name.UTF8String,
                    [NSThread currentThread].description.UTF8String);
            [NSThread sleepForTimeInterval:2]; // 模拟耗时
            if (self.isCancelled) {
                printf("isCancelled: %s\n",self.name.UTF8String);
                return;
            } else {
                dispatch_async(dispatch_get_main_queue(), ^{
                    printf("do something at main queue");
                });
            }
        }
    }
    @catch (NSException *exception) {
        NSLog(@"%@",exception);
    }
}
@end
```

上面的代码中，多次检查`isCancelled`。其实，operation 开始执行之后，会一直执行任务直到完成，或者显式地取消操作。取消可能在任何时候发生，甚至在 operation 执行之前。尽管 NSOperation 提供了一个方法，让应用取消一个操作，但是识别出取消事件则是你的事情。如果 operation 直接终止，可能无法回收所有已分配的内存或资源。因此 operation 对象需要检测取消事件，并优雅地退出执行。operation 对象定期地调用 isCancelled 方法，如果返回 YES(表示已取消)，则立即退出执行。不管是自定义NSOperation 子类，还是使用系统提供的两个具体子类，都需要支持取消。isCancelled 方法本身非常轻量，可以频繁地调用而不产生大的性能损失。以下地方可能需要调用isCancelled:

* 在执行任何实际的工作之前
* 在循环的每次迭代过程中，如果每个迭代相对较长可能需要调用多次
* 代码中相对比较容易中止操作的任何地方

使用`NonConcurrentOperation`:

```objective-c
- (void)viewDidLoad {
    [super viewDidLoad];
    
    NonConcurrentOperation *nonConcurrentOperation =
            [[NonConcurrentOperation alloc] initWithName:@"onevlin.com"];
    nonConcurrentOperation.completionBlock = ^ {
        NSLog(@"=== completedBlcok ===");
    };

    dispatch_queue_t global_queue =
            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    dispatch_time_t cancelTime =
            dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC));
    // 1 秒后cancel
    dispatch_after(cancelTime, global_queue, ^{
        [nonConcurrentOperation cancel];
    });
    [nonConcurrentOperation start];

}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operaiton_nonConcurrentOperation.png)

如果没有调用`[nonConcurrentOperation cancel]`那么定义的operation最后会打印`"do something at main queue"`:

```objective-c
//[nonConcurrentOperation cancel];
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operaiton_nonConcurrentOperation_nonCancel.png)

#### **自定义并发的operation**

Operation 对象默认按同步方式执行，也就是在调用 start 方法的那个线程中直接执行。由于 operation queue 为非并发 operation 提供了线程支持，对应用来说，多数 operations 仍然是异步执行的。但是如果你希望手工执行 operations，而且仍然希望能够异步执行操作，你就必须采取适当的措施，通过定义 operation 对象为并发操作来实现。

|          方法           | 描述                                                         |
| :---------------------: | ------------------------------------------------------------ |
|          start          | (必须)所有并发操作都必须覆盖这个方法，以自定义的实现替换默认行为。手动执行一个操作时，你会调用 start 方法。因此你对这个方法的实现是操作的起点，设置一个线程或其它执行环境，来执行你的任务。你的实现在任何时候都绝对不能调用 super。 |
|          main           | (可选)这个方法通常用来实现 operation 对象相关联的任务。尽管你可以在 start 方法中执行任务，使用 main 来实现任务可以让你的代码更加清晰地分离设置和任务代码 |
| isExecuting、isFinished | (必须)并发操作负责设置自己的执行环境，并向外部 client 报告执行环境的状态。因此并发操作必须维护某些状态信息，以知道是否正在执行任务，是否已经完成任务。使用这两个方法报告自己的状态。这两个方法的实现必须能够在其它多个线程中同时调用。另外这些方法报告的状态变化时，还需要为相应的 key path 产生适当的 KVO通知。 |
|      isConcurrent       | (必须)标识一个操作是否并发 operation，覆盖这个方法并返回 YES |

下面定义一个并发的operation`ConcurrentOperation`。

```objective-c
@implementation ConcurrentOperation

- (instancetype)initWithName:(NSString *)name {
    if (self = [super init]) {
        self.name = name;
        executing = NO;
        finished = NO;
    }
    return self;
}

- (BOOL)isConcurrent {
    return YES;
}
- (BOOL)isExecuting {
    return executing;
}
- (BOOL)isFinished {
    return finished;
}

-(void)start {

    //第一步就要检测是否被取消了，如果取消了，要实现相应的KVO
    if ([self isCancelled]) {

        [self willChangeValueForKey:@"isFinished"];
        finished = YES;
        [self didChangeValueForKey:@"isFinished"];
        return;
    }

    //如果没被取消，开始执行任务
    [self willChangeValueForKey:@"isExecuting"];

    [NSThread detachNewThreadSelector:@selector(main)
                             toTarget:self withObject:nil];
    executing = YES;
    [self didChangeValueForKey:@"isExecuting"];
}

- (void)main {
    @try {
        @autoreleasepool {
            NSLog(@"==== 执行自定义非并发NSOperation ====");
            if (self.isCancelled) {
                printf("isCancelled: %s\n",self.name.UTF8String);
                return;
            };
            printf("name: %s , thread: %s\n",
                    self.name.UTF8String,
                    [NSThread currentThread].description.UTF8String);
            [NSThread sleepForTimeInterval:2]; // 模拟耗时
            if (self.isCancelled) {
                printf("isCancelled: %s\n",self.name.UTF8String);
                return;
            } else {
                //任务执行完成后要实现相应的KVO
                [self willChangeValueForKey:@"isFinished"];
                [self willChangeValueForKey:@"isExecuting"];

                executing = NO;
                finished = YES;

                [self didChangeValueForKey:@"isExecuting"];
                [self didChangeValueForKey:@"isFinished"];
                dispatch_async(dispatch_get_main_queue(), ^{
                    printf("do something at main queue\n");
                });
            }
        }
    }
    @catch (NSException *exception) {
        NSLog(@"%@",exception);
    }
}
```

使用`ConcurrentOperation`:

```c
- (void)viewDidLoad {
    [super viewDidLoad];

    ConcurrentOperation *concurrentOperation =
            [[ConcurrentOperation alloc] initWithName:@"onevlin.com"];
    concurrentOperation.completionBlock = ^ {
        NSLog(@"=== completedBlcok ===");
    };

    [concurrentOperation start];

}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/operaton_ConcurrentOperation.png)

可以看到`number = 3`operation在调用`[concurrentOperation start]`任务也其他线程中异步执行。
​			

### What Next？

iOS开发中并发编程主要使用的两部分内容GCD和NSOperationQueue到这里也算是有个小结尾了。以后遇到有趣的内容，再补充进来。接下来的几篇文章会看看UI层相关的内容，我们一起进步吧，加油。
​	