---
layout: post
title: RxSwift:Combine Operators(2)
subtitle: 
author: JackLin
date: 2018-02-09 00:39:47 +0800
---



### 前言

在Combine Operators(1)中，我们了解到了如何将Observable合并。这一次我们一起实践一下如何合并多个Observables中的事件。老规矩:

> **Tips:为了打印方便直观，这里会先定义一个example函数，将事例代码都放到example里面执行**

```swift
func example(_ description: String,
             action: () -> Void) {
    print("================== \(description) ==================")
    action()
}
```

### combineLatest

多个Observabls中的**当前事件**合并成一个事件

```swift
example("combineLatest") {
    let bag = DisposeBag()
    let sequenceA = PublishSubject<String>()
    let sequenceB = PublishSubject<String>()
    let suquence = Observable.combineLatest(sequenceA, sequenceB) { a, b in
         a + b
    }
    suquence.subscribe(onNext: {
        dump($0)
    }).disposed(by: bag)
    
    sequenceA.onNext("A1")
    sequenceB.onNext("B1")
    sequenceA.onNext("A2")
    sequenceA.onNext("A3")
    sequenceB.onNext("B2")
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswfit_combineLast_log.png)

上面使用了combineLatest合并了两个Observable中的事件，具体的合并定义在一个closure中。实际上，`combineLatest`不止接受2个Observables，最多可以可以给它传递8个。

> Combines up to 8 source Observable sequences into a single new Observable sequence, and will begin emitting from the combined Observable sequence the latest elements of each source Observable sequence once all source sequences have emitted at least one element, and also when any of the source Observable sequences emits a new element

combineLatest的合并规则可以归纳为两点：

1.  **只有在每一个Sub-observable中都发生过一个事件之后**，`combineLatest`才会执行我们定义的closure。
2.  每当每个**Sub-observable**中有新的事件更新时，都会从每个**Sub-observable**中取出最后一个事件合并成一个新的事件。

我们结合序列图来看一下：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswfit_combineLast_process.png)

在这个图中：

1. 序列sequenceA发送A1事件时sequenceB中没有任何事件，因此不会有合并操作。

2. 序列sequenceB发送事件B1时，2个序列都有了事件，取两个序列中的最新事件进行a+b合并得到：A1B1

3. 序列sequenceA发送事件A2时，取两个序列中的最新事件进行a+b合并得到：A2B1

4. 序列sequenceA发送事件A3时，取两个序列中的最新事件进行a+b合并得到：A3B1

5. 序列sequenceB发送事件A2时，取两个序列中的最新事件进行a+b合并得到：A3B2

   ​

经过combineLatest得到合并后的sequence为：*A1B1->A2B1->A3B1->A3B2*

**我们也可以使用一个集合类型来得到一个combineLatest的事件序列：**

```swift
example("combineLatest") {
    let disposeBag = DisposeBag()
    let stringObservable = Observable.just("❤️")
    let fruitObservable = Observable.from(["🍎", "🍐", "🍊"])
    let animalObservable = Observable.of("🐶", "🐱", "🐭", "🐹")
    
    Observable.combineLatest([stringObservable, fruitObservable, animalObservable]) {
        "\($0[0]) \($0[1]) \($0[2])"
        }
        .subscribe(onNext: { print($0) })
        .disposed(by: disposeBag)
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswfit_combineLast_collection_log.png)

> Because the combineLatest variant that takes a collection passes an array of values to the selector function, it requires that all source Observable sequences are of the same type.
>
> 我们在合并的clousre里，会收到一个`Array<T>`参数，其中`T`就是Sub-observables中的事件类型。因此，这种数组参数的用法，要求`combineLatest`的所有Sub-observables的事件类型都相同。如果需要合并合并事件类型不同的Sub-observables那么还是使用`Observable.combineLatest(sequenceA, sequenceB)`的方法来合并吧。

关于`combineLatest`的生命周期也可以归纳为两点：

**只有所有的Sub-observable都完成之后，合并后的Observable才会发生Completed事件。**

对第一个example中的事件更改，让sequenceA在事件A3后便完成。sequenceB在B2事件完成，并且再添加一个事件B3:

```swift
sequenceA.onNext("A1")
sequenceB.onNext("B1")
sequenceA.onNext("A2")
sequenceA.onNext("A3")
sequenceA.onCompleted()
sequenceB.onNext("B2")
sequenceB.onCompleted()
sequenceB.onNext("B3")
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswfit_combineLast_log.png)

可以看到和最开始的合并结果一样，因为事件整个合并序列是在sequenceB.onCompleted()后才结束。

**如果在合并的过程中有Sub-observable发生Error事件，`combineLatest`合成的Observable就会立即结束**

对上面的事件进一步更改：让sequenceA在发送A2后发生错误，这里可以随意定义个错误：

```swift
sequenceA.onNext("A1")
sequenceB.onNext("B1")
sequenceA.onNext("A2")
sequenceA.onError(exampleError.combineLatest)
sequenceA.onNext("A3")
sequenceA.onCompleted()
sequenceB.onNext("B2")
sequenceB.onCompleted()
sequenceB.onNext("B3")
```

```swift
enum exampleError: Error {
    case combineLatest
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswfit_combineLast_error.png)

AsequenceA.onError(error)发生已经合并了事件:A1B1,A2B1,在AsequenceA.onError(error)发生后`combineLatest`合成的Observable就会立即结束。所以上面我们仅仅订阅到事件在error发生前的事件序列:

*A1B1->A2B1*



### zip

zip的用法和`combineLatest`几乎是相同的，我们可以把之前的合并代码改成这样试一下(除了改用`zip` operators之外，没有任何变化)：

```swift
example("combineLatest") {
    let bag = DisposeBag()
    let sequenceA = PublishSubject<String>()
    let sequenceB = PublishSubject<String>()
    let suquence = Observable.zip(sequenceA, sequenceB) { a, b in
         a + b
    }
    suquence.subscribe(onNext: {
        dump($0)
    }).disposed(by: bag)

    sequenceA.onNext("A1")
    sequenceB.onNext("B1")
    sequenceA.onNext("A2")
    sequenceA.onNext("A3")
    sequenceB.onNext("B2")
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_zip_log.png)

> Combines up to 8 source Observable sequences into a single new Observable sequence, and will emit from the combined Observable sequence the elements from each of the source Observable sequences at the corresponding index.

同combineLatest一样最多可以可以合并8个Observable，但是有一些几点需要注意区别:

1. 在每次combine后Sub-observable的事件相当于被消耗完毕。必须所有的Sub-observable都有更新事件才会进行下一次的combine。


2. zip合成的Observable中，其中任何一个Sub-observable发生了Completed事件，整个合成的Observable就完成了。(结合1应该和容易理解，因为任何一个Sub-observable不进行事件更新，那么就不会再有combine了，所以就完成)

对于上面两点我们用序列图看看：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_zip_process.png)

在这个图中：

1. 序列sequenceA发送A1事件时sequenceB中没有任何事件，因此不会有合并操作。
2. 序列sequenceB发送事件B1时，2个序列都有了事件，取两个序列中的最新事件进行a+b合并得到：A1B1。在合并后，sequenceA和sequenceB的事件都相当于被消费掉了。
3. 序列sequenceA发送事件A2时，sequenceB的事件没有更新所以没有合并操作。
4. 序列sequenceA发送事件A3时，同样sequenceB的事件没有更新所以没有合并操作。
5. 序列sequenceB发送事件AB时，连个序列都有更新的事件，取两个序列中的最新事件进行a+b合并得到：A3B2
6. 合并后的sequence为：*A1B1->A3B2*

### 最后

晚点我们一起探讨Observables之间更复杂的关系，如何根据事件，在多个Observables之间进行跳转。