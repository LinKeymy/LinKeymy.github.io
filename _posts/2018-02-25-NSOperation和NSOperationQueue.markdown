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

### 队列的取消、暂停、恢复

