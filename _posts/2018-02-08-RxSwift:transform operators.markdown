---
layout: post
title: RxSwift:Transform Operators
subtitle: 
author: JackLin
date: 2018-02-08 17:35:58 +0800
---

### 前言

前面我们学习了RxSwift的Filter Operators，很有意思吧，这篇文章主要介绍RxSwift的Transform Operators，这算是RxSwift中最重要的操作符了。废话不多说，一起看看吧。

* **Tips**

> 为了打印方便直观，这里会先定义一个example函数，将事例代码都放到example里面执行

```swift
func example(_ description: String,
             action: () -> Void) {
    print("================== \(description) ==================")
    action()
}
```

#### toArray

把`Observable<T>`中所有的事件值，**在订阅的时候**，打包成一个`Array<T>`返回给订阅者

```swfit
example("toArray") {
    let bag = DisposeBag()
    Observable.of("a", "b", "c")
            .toArray()
            .subscribe(onNext: {
                print(type(of: $0))
                print($0)
            }).disposed(by: bag)
}
```

上面的序列`"a", "b", "c"`被`toArray`打包到一个数组里面，然后一次性发送这个发送是发生在subscribe的时候：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_toArray.png)

第三次强调一下`toArray`是对已有事件序列的操作。对于在订阅行为发生后产生的事件就不起作用了。比如:

```swift
example("toArray") {
    let bag = DisposeBag()
    let numbers = PublishSubject<Int>()

    numbers.asObservable()
            .toArray()
            .subscribe(onNext: {
                print($0)
            }).disposed(by: bag)
    numbers.onNext(1)
    numbers.onNext(2)
    numbers.onNext(3)
}
```

这种情况`toArray`操作的时候没有任何事件。所有subscribe也订阅不到任何的事件：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_toArray_wrong.png)

#### scan

scan的作用是给它设定一个初始值之后，然后在给定clourse对Observable序列中的每一个事件进行运算，初始值作为第一次运算的操作数倍传入clourse，每次都返回运算的结果替换初始值用于下次运算的操作数。

```swift
example("scan") {
    let bag = DisposeBag()
    Observable.of("w", "o", "r","l","d").scan("Hello ") {
        return $0 + $1
    }.subscribe {
        print($0)
    }
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_scan.png)



上面图中展示的那样，**和toArray不同的是，scan在Observable每次有事件的时候都会执行**。或许下面的例子更加明显看到区别：

```swift
example("scan") {
    let bag = DisposeBag()
    let hello = PublishSubject<String>()

    hello.asObservable().scan("Hello ") {
        return $0 + $1
    }.subscribe {
        print($0)
    }.disposed(by: bag)

    hello.onNext("world")
    hello.onNext(" Alin")
}
```

订阅到了`Hello world` 和 `Hello world Alin` ，每次有事件发生时，`scan`都会进行将前面的结果用于clourse订阅的运算中。

> scan和Array的reduce很像

### map

map用于转换事件，它作用于序列中的每个事件，但是每次对事件的操作都是独立的，不依赖于前面操作的结果，这是map区别于scan的很重要的一点。`map`接受一个closure，而这个closure的参数，就是原Observable中的事件值。

```swift
example("map") {
    let bag = DisposeBag()
    Observable.of(1, 2 , 3).map {
        value in value * value
    }.subscribe(onNext: {
        print($0)
    }).disposed(by: bag)
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_map.png)

对于subject也是一样：

```swift
example("map") {
    let bag = DisposeBag()
    let number = PublishSubject<Int>()
    number.map {
        value in value * value
    }.subscribe(onNext: {
        print($0)
    }).disposed(by: bag)
    number.onNext(3)
    number.onNext(5)
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_map_onnext.png)

> 除了RxSwift带来的异步操作，这里的map和Array的map也是很像



#### flatMap

flatMap是一个很重要，但是又很让人迷惑费解的操作符。

[官方](http://reactivex.io/documentation/operators/flatmap.html)给出的作用说明:

**transform the items emitted by an Observable into Observables, then flatten the emissions from those into a single Observable**

> 1.  transform items into observables
> 2. flatten emissions into a single Observable

附加的解释是(这里我拆分两段对应上的1和2):

1. FlatMap operator transforms an Observable by applying a function that you specify to each item emitted by the source Observable, where that function returns an Observable that itself emits items. 
2. FlatMap then merges the emissions of these resulting Observables, emitting these merged results as its own sequence.

> FlatMap使用一个指定的function作用于source Observable emitted出来的每一个item(element)并返回，每个item都会返回一个对应的Observable。(trans items into observables),然后将上一步的Observables emitted 的所有的items(elements)拿出来放到一个事件序列里面。

拉扯了这么多，代码呢？，如下：

```swift
example("flatMap") {
 let bag = DisposeBag()
 let ob = Observable.of(["a","b"],["c","d"])
    ob.flatMap { Observable.of($0.first ?? "", $0.last ?? "") }
      .subscribe( onNext:{ print($0) } )
      .disposed(by: bag)
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_flatmap.png)

上面的结果经历了前面的两个过程:

1. 原ob序列的每个item(element)`["a","b"]`和`["c","d"]`都会被Observable.of($0.first ?? "", $0.last ?? "")操作变成对应的`Observable.of("a","b")`和`Observable.of("c","d")`这一步就是`transform the items emitted by an Observable into Observables`将一个**Observable**的**items**转换成**Observables**
2. 将第一步得到的**Observables**`Observable.of("a","b")`和`Observable.of("c","d")`的序列拿出来：`Observable.of("a","b")` 发送事件 "a","b"，Observable.of("c","d")发送事件:"c","d"。将得到的所有事件序列都放到一个事件序列里面得到: "a","b"",c","d"。

这里附加官方**Rx.playground**的例子(觉得还是我上面的例子比较容易理解,自恋了):

```swift
example("flatMap") {

    let bag = DisposeBag()

    struct Player {
        var score: Variable<Int>
    }

    let jack = Player(score: Variable(80))
    let rose = Player(score: Variable(90))

    let player = Variable(jack)

    player.asObservable()
            .flatMap { $0.score.asObservable() }
            .subscribe(onNext: { print($0) })
            .disposed(by: bag)

    jack.score.value = 85
    player.value = rose
    rose.score.value = 95
    rose.score.value = 100
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_transform_Rx.playground.png)

经过上面例子的相信说明，这里的结果应该不难理解，就不在唠叨了。

> **tips:**从playe的Observabler变成score的Observablers。没有flatmap是从player中订阅到jack，但是有了flatmap变成订阅jack的scores(jack可以一直产生很多的分数)了。rose进来经过同样处理流程被添加到一个序列中，也就是会同时订阅到jack和rose的分数。



#### flatMapLatest

另外一个和`flatMap`类似的operator是`flatMapLatest`。当原序列中有新事件发生的时候，`flatMapLatest`就会自动取消上一个事件的订阅，然后转换到新事件的订阅。而`flatMap`则会保持原序列中的所有事件订阅。将上面的`flatMap`换成`flatMapLatest`:

```swift
example("flatMapLatest") {

    let bag = DisposeBag()

    struct Player {
        var score: Variable<Int>
    }

    let jack = Player(score: Variable(80))
    let rose = Player(score: Variable(90))

    let player = Variable(jack)

    player.asObservable()
            .flatMapLatest { $0.score.asObservable() }
            .subscribe(onNext: { print($0) })
            .disposed(by: bag)

    jack.score.value = 85
    player.value = rose
    rose.score.value = 95
    jack.score.value = 100  // 这里是jack的分数事件
}
```

将最后一个rose的分数改成Jack的分数，最后看看执行结果:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_transform_flatmapLast.png)

这次，在开始会订阅到player，jack的score事件，但是player的变成rose后就不在订阅jack的score事件了，因为flatMapLatest只会处理订阅最新唯一的source Observable。



### 最后

我们不妨思考一个问题。在什么情况下需要使用`flatMap`呢？为什么要把一个序列中的事件，变成另外一个事件序列呢？简单来说，因为现实中很多事件都是异步发生的，而并不是像`Observable.of`创建的看起来像集合这样的。因此，当我们需要对异步发生的事件序列进行变换的时候，就需要订阅原来的事件序列，对异步发生的事件有所察觉。其中，网络编程就是一个最典型的例子。为了在请求一个网络资源后，根据服务器返回的结果对原事件序列进行变换，`flatMap`就是最好的选择。