---
layout: post
title: RxSwift:Combine Operators(3)
subtitle: 
author: JackLin
date: 2018-02-13 18:43:30 +0800
---



### 前言

在平时编码过程过程中，我们会遇到一类问题，两个Observable中的内容，在逻辑上，是有关联的。比如验证用户信息时，在点击确认按钮的时候要拿到用户输入的用户名信息，这里就有两个序列：一个是确认按钮的点击事件序列；另一个是UITextfiled值的序列。RxSwift提供了一种方式，让我们可以在不同的Observable中进行切换。这次我们就是play一下这个特性吧。还是老方法：

> **Tips:为了打印方便直观，这里会先定义一个example函数，将事例代码都放到example里面执行**

```swift
func example(_ description: String,
             action: () -> Void) {
    print("================== \(description) ==================")
    action()
}
```

### withLatestFrom

`sequenceA.withLatestFrom(sequenceB)`: 表示当序列sequenceA发生事件时，我们订阅到了sequenceB的事件。下面我们模拟一下当用户点击确认的时候获取用户的用户名。

首先定义两个事件序列`userNameSequence`和`confirmSequence`分别代表了用户名输入事件和确认按钮点击事件：

```swift
let userNameSequence = BehaviorSubject<String>(value:"empty")
let confirmSequence = PublishSubject<Void>()
```

然后使用`withLatestFrom`将两个事件关联：

```swift
let suquence = confirmSequence.withLatestFrom(userNameSequence)
```

然后订阅关联后的序列事件，完整的代码如下：

```swift
example("withLatestFrom") {
    let bag = DisposeBag()
    let userNameSequence = BehaviorSubject<String>(value:"empty")
    let confirmSequence = PublishSubject<Void>()
    let suquence = confirmSequence.withLatestFrom(userNameSequence)
    suquence.subscribe(
        onNext: {
            dump($0)},
        onDisposed: {
            print("suquence onDisposed")}
        ).disposed(by: bag)
    confirmSequence.onNext(())
    userNameSequence.onNext("alin")
    confirmSequence.onNext(())
    userNameSequence.onNext("nice alin")
    confirmSequence.onNext(())
    confirmSequence.onNext(())
}
```

然后看看运行后的订阅结果：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_latestFrom_log.png)

首先`userNameSequence`有一个默认的事件empty。我们每次confirmSequence.onNext(())都会订阅到`userNameSequence`的最新事件，这样就将序列`userNameSequence`的事件关联到了`confirmSequence`序列。

### switchLatest

` switchLatest`给我的第一个感觉就像是在`git`中切换分支。使用可以`switchLatest`多个Observables之间进行跳转。让你想从订阅序列A事件，切换到订阅序列B来得易如反掌。不过我还是喜欢用git的工作分支作为类比,或许不太恰当。

我们定义了两个事件序列`develop`和`develop`

```swift
let release = PublishSubject<String>()
let develop = PublishSubject<String>()
```

同时定义了一个`working`它的事件序列是**Observable**

```swift
let working = PublishSubject<Observable<String>>()
```

在订阅`working`前使用`switchLatest()`这样就能让后面的订阅使用`onNext`进行切换订阅的序列;

```swift
working.switchLatest().subscribe(
     onNext: {
          dump($0)
      }).disposed(by: bag)
```

下面看看完整的代码和订阅到的事件：

```
example("switchLatest") {

    let bag = DisposeBag()
    let release = PublishSubject<String>()
    let develop = PublishSubject<String>()
    let working = PublishSubject<Observable<String>>()

    working.switchLatest().subscribe(
        onNext: {
            dump($0)
        }).disposed(by: bag)
    
    working.onNext(develop)
    release.onNext("release-commit-00")
    develop.onNext("develop-commit-00")
    working.onNext(release)
    release.onNext("release-commit-01")
    develop.onNext("develop-commit-01")

}
```

使用`working.onNext(develop)`表示working的事件序列的订阅切换到`develop`分支。此时发送下面两个事件，我们订阅到是`develop`的事件。

```swift
release.onNext("release-commit-00")
develop.onNext("develop-commit-00")
```

  使用`working.onNext(develop)`表示working的事件序列的订阅切换到`develop`分支。此时发送下面两个事件,我们订阅到是`release`的事件。

```swift
release.onNext("release-commit-01")
develop.onNext("develop-commit-01")
```

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_combine_switchLatest_log.png)

以上，就是在不同的Observable之间切换事件的用法.

### 最后

前面分享了合并序列本身、合并序列事件以及在事件序列之间切换的方法。他们都有一个共同的名字叫做：**combine operators**。这里Combine Operators的介绍就暂告一段了。