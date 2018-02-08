---
layout: post
title: RxSwift:Observble
subtitle: 
author: JackLin
date: 2018-02-08 15:52:56 +0800
---



### RxSwift的Observable资源被回收的情况

- Observable本身的事件已经完全emit完毕
- Observable没有任何subscribe的时候，也就是所有的disposable都已经disposed

在RxSwift中因为使用subscribe来表示订阅Observable中emit出来的序列事件，因此衍生出来对Observable对象资源的管理方式很直观。毕竟Observable是用来emit事件的，对于第一点既然Observable没有事件emit就没有存在的必要了，对于第二点Observable在其作用域没有任何订阅者，那么也是一样没有存在的必要了。

### Subject

Subject,可以同时作为Observable和Observer.对于一个普通的Observable只能由Observable内部是预定好一些事件，然后emit.

```swift
let obaleOf = Observable.of("1","2","3","4","5","6","7","8","9","10").subscribe {
    print($0)
}
```

但是Subject提供通道可以从不获取事件(也可以理解它可以订阅事件)，然后将事件发送给它的订阅者。

#### PublishSubject

`PublishSubject`就像个出版社，首先它是一个Observer，到处收集内容，，然后它也是一个Observable，将订阅者到的内容发送给它的订阅者。`PublishSubject`执行的是“会员制”，它只会把最新的消息通知给消息发生之前的订阅者。如果要订阅到某事件必须在`PublishSubject`发送消息前订阅。

#### BehaviorSubject

`BehaviorSubject`和`PublishSubject`功能上是一样的。不同的是它带有一个事件默认的订阅事件缓存。在创建的时候就必须提供一个默认的事件。它订阅到新的事件后，会将最新的事件替换默认的事件。这样只要订阅都会先收到`BehaviorSubject`最后一次发送的事件。

#### ReplaySubject

`ReplaySubject`结合了`BehaviorSubject`和`BehaviorSubject`的特性。

- `ReplaySubject`没有默认消息，订阅空的`ReplaySubject`不会收到任何消息；这时和`PublishSubject`一样。
- `ReplaySubject`自带一个可定义的缓冲区，当有订阅者订阅的时候，它会向订阅者发送缓冲区内的所有消息；如果将缓存区设置为1，除了没有默认消息，功能上和`BehaviorSubject`很相似。

### Variable

除了事件序列之外，在平时的编程中我们还经常需遇到一类场景，就是需要某个值是有“响应式”特性的，例如可以通过设置某个变量的值来动态控制按钮是否禁用，是否显示某些内容等，这个可以在一定程度长替换KVO的使用。为了方便这个操作，RxSwift提供了一个特殊的subject，叫做`Variable`。

`Variable`只用来表达一个“响应式”值的语义，因此，它有以下两点性质：

- 没有`.error`事件；
- 无需手动给它发送`.complete`事件表示完成；

对于这两点可以这样理解：`Variable`就是将需要监听的对象包装起来，然后使用asObservable将经过包装后每次更新都从新发送，唯一的目的就是用于响应变化，因此也就没有error和completed的概念了。