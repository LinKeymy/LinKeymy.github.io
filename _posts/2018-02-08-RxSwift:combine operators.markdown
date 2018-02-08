---
layout: post
title: RxSwift:Combine Operators
subtitle: 
author: JackLin
date: 2018-02-08 22:49:41 +0800
---

### 前言

Filter operators、Transform operators都是和单个Observable相关的。很多时候，我们需要把多个事件序列合并起来表达某个现实中的情况。为此，RxSwift提供了另外一大类operators完成这个工作，它们叫做**combine operators**。点上咖啡，准备好专辑，我们一起看看这类神奇的operators吧。

> **Tips:为了打印方便直观，这里会先定义一个example函数，将事例代码都放到example里面执行**

```swift
func example(_ description: String,
             action: () -> Void) {
    print("================== \(description) ==================")
    action()
}
```

#### startWith

`startWith`为特定的事件序列，添加前置条件。比如每次呼叫(订阅)*ALin*的时候都先说*Hello*:

```Swift
example("startWith") {
    let bag = DisposeBag()
    let hello = PublishSubject<String>()
    hello.startWith("Hello")
        .subscribe (
            onNext:{ print($0) }
    ).disposed(by: bag)

    hello.onNext("ALin")
}
```

执行一下，在控制台看到两个事件：第一个事件值是*Hello*，也就是`startWith`发生的事件；第二个事件值是*ALIn。

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_startWith.png)

> 需要注意的是**startWith中的事件值的类型，和它后续的事件值类型，必须是相同的**。上面的例是`String`，否则，会导致编译错误。



#### concat

`concat`以把两个并行的Observable合并起来串行处理,比如下面使用concat串联sequenceA和sequenceB，实现**先处理完queueA中的事件之后，再开始处理queueB中的事件**

```swift
example("concat") {
    let bag = DisposeBag()
    let sequenceA = PublishSubject<String>()
    let sequenceB = PublishSubject<String>()
    let sequence = Observable
            .concat([sequenceA.asObservable(), sequenceB.asObservable()])
    sequence.subscribe(onNext: {
        dump($0)
    }, onCompleted: {
        print("Completed")
    }, onDisposed:  {
        print("Disposed")
    }).disposed(by:bag)

    sequenceA.onNext("A1")
    sequenceA.onNext("A2")
    sequenceB.onNext("B1")
    sequenceA.onCompleted()
    sequenceB.onNext("B2")
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_concat_01.png)

从执行结果可以看到，我们能订阅到了*A1 -> A2  -> B2 -> Disposed*事件。由于*B1*事件在发送的时候`sequenceA`还没有处理完成所以被忽略。最后example执行完后bag为nil，最后也就订阅到了Disposed的事件。你会发现这里为什么没有订阅到Completed事件呢？这是因为，**只有concat中所有Sub observables都完成时，合成后的Observable才会完成**。因此，要想订阅到*Completed*把之前的事件序列改成下面这样就好了：

```
sequenceA.onNext("A1")
sequenceA.onNext("A2")
sequenceB.onNext("B1")
sequenceA.onCompleted()
sequenceB.onNext("B2")
sequenceB.onCompleted()
```

执行检查结果，符合预期:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_concat_completed.png)

> *Tips:* 除了接受数组的全局`concat` operator，Observable自身也有一个`concat`方法，允许我们合并两个Observables：let sequence = sequenceA.concat(sequenceB.asObservable())

#### merge

世界往往是平衡互补的。既然有了‘’串行以‘合并，那么也少不了“并行”合并所有的Observable。`merge` operator就是用来实现“并行”合并。只要合并进来的Observable中有事件发生，我们就可以订阅到，而无需等待前置的Observable结束。请把之前合并Observable的代码改成这样：

```swift
let sequence = Observable
    .merge([sequenceA.asObservable(), sequenceB.asObservable()])
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_merge.png)

执行一下，我们就会单纯的按照事件发生的顺序，依次订阅到*A1 -> A2 -> B1 -> A3 -> Completed -> Disposed*事件了。

#### merge的最大并行订阅量maxConcurrent

`merge`拥有并行订阅的特性，和iOS的线程最大并发量处理类似，我们可以控制合并的过程中同时订阅的Sub-observable数量，默认条件下，`merge`当然就是同时订阅合并进来的所有Sub-observable。

```Swift
example("startWith") {
    let bag = DisposeBag()
    let sequenceA = PublishSubject<String>()
    let sequenceB = PublishSubject<String>()
    let sequenceC = PublishSubject<String>()
    let sequence = Observable.of(sequenceA.asObservable(),
                                 sequenceB.asObservable(),
                                  sequenceC.asObserver())
            .merge(maxConcurrent: 2)
    sequence.subscribe(onNext: {
        dump($0)
    }, onCompleted: {
        print("Completed")
    }, onDisposed:  {
        print("Disposed")
    }).disposed(by:bag)

    sequenceA.onNext("A1")
    sequenceB.onNext("B1")
    sequenceC.onNext("C1")
    sequenceA.onCompleted()
    sequenceB.onNext("B2")
    sequenceC.onNext("C2")
    sequenceB.onCompleted()
    sequenceC.onCompleted()
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_merge_maxCurrent.png)

我们指定最大并发订阅数为2，所以sequenceC.onNext("C1")的时候没有被订阅到，因为这时候sequenceA，sequenceB被订阅中。直到sequenceA.onCompleted()后就可以订阅sequenceC的消息了，这样C2就被订阅到。

### 下集预告

下一篇，我们来看如何合并Observable中的事件！