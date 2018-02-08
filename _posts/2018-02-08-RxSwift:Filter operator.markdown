---
layout: post
title: RxSwift:Filter Operator
subtitle: 
author: JackLin
date: 2018-02-08 15:55:44 +0800
---

### 前言

为了高效语义正确地使用Rxswift首先要了解它常见的对事件队列的操作符，合理准确地使用这类操作符往往会使开发事半功倍。下面就从过滤事件的操作符开始。过滤操作符可以细分为两类：

1. 忽略事件的操作符：用于过滤掉不关心的事件(漏斗过滤)
2. 选择事件的操作符：选择性订阅需要的事件(夹子夹取)

从文字表述上可以看出，他们的目的都是一样的：就是只关心想订阅的事件。

### 忽略事件操作符

从忽略全部事件到自定义指定事件，RxSwift提供了多种operators，先看看RxSwift的忽略(过滤)事件的operator

#### ignoreElements: 忽略所有的next事件

一次性忽略掉所有的`.next`事件，如图，使用了ignoreElements后值接受到了complected事件

![有帮助的截图]({{ site.url }}/assets/postsImages/exmaple_ignoreElements.png)



#### skip(n): 跳过前面特定个数的next事件

选择性忽略事件序列中前面特定个数的`.next`事件

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_ignore_skip.png)

#### skipWhile(clourse) / skipUntil(o): 根据条件选择性忽略

`skipWhile`从开始while的条件满足时都会skip，知道遇到一个不满足给定条件的事件后就不在skip。正如下面显示，第一个Task1 != Task2 会skip跳过,但是遇到Task2 != Task2不满足条件后就开始接受事件，所以最后一个Task1事件被接收。

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_ignore_skipwhile.png)

`skipUntil`和`skipWhile`使用的概念上非常相似，只是判断依据的对象不同`skipWhile`是根据订阅的事件事件判断是非跳过，而是`skipUntil`使用另外一个事件序列中的事件,如果另外一个序列的事件发送了，那么久开始订阅接受事件。

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_ignore_skipuntil.png)

对比`skipWhile`和`skipUntil`很有意思，好像在公司上班，skipWhile不过老板有没有来，只负责处理自己该做的份内的事情。`skipUntil`则是一直都不做事，直到老板来了才开始做事。😄，有意思～

#### distinctUntilChanged

`distinctUntilChanged`忽略序列中连续重复的事件：

> 特别要注意时连续且重复的事件，下面也可以看到后面还是会订阅到重复但是不连续的Task1.

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_ignore_skipuntil_distinctUntilChanged.png)

### 获取事件操作符

#### elementAt(n)

是选择序列中的第n个事件

```swift
import RxSwift
import Foundation

func example(_ description: String,
             action: () -> Void) {
    print("================== \(description) ==================")
    action()
}

example("elementAt") {
    let tasks = PublishSubject<String>()
    let bag = DisposeBag()

    tasks.elementAt(1)
            .subscribe {
                print($0)
            }
            .disposed(by: bag)
    tasks.onNext("Task1")
    tasks.onNext("Task2")
    tasks.onNext("Tas32")
    tasks.onNext("Task4")
    tasks.onCompleted()
}
```

elementAt(n)的索引和数组下标一样从0开始，这样就选择了Task2:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_elementAt.png)

#### filter(closure)

用一个closure设置选择事件的标准，它会选择序列中所有满足条件的元素。例如，我们订阅值是`Task3`的事件：

```swift
tasks.filter{ return $0 == "Task3" }
    .subscribe {
        print($0)
    }
    .disposed(by: bag)
```

检查订阅结果:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_filter.png)

#### take(n)

选择订阅序列中的前`n`个事件，例如要订阅前面3个事件:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_take.png)

#### takeWhile / takeWhileWithIndex

`takeWhile`用一个closure来指定“只要条件为true就一直订阅下去”，只要遇到false就立刻停止不在订阅,例如只要遇到Task3就停止订阅:

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_takewhile.png)

使用`takeWhile`比较容易犯一个逻辑错误就是：在takeWhile里提供的是订阅的终止条件，比如上面我们希望遇到Task3就终止订阅。有时候会迷糊写下：

```swift
tasks.takeWhile{ return $0 == "Task3"}
    .subscribe {
        print($0)
    }
    .disposed(by: bag)
```

上面的代码会得到什么结果呢？实际上只能订阅到`.completed`。因为，当匹配到第一个事件的时候，`"Task1" == "Task3"`是`false`，所以订阅就结束了。

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_takewhile_wrong.png)

`takeWhile`的使用姿势应该是: while(true) take while(false) stop

`takeWhileWithIndex`是一个功能和`takeWhile`类似的operator，只是在它的closure里，可以同时访问到事件值和事件在队列中的索引，这样我们不仅可以约束订阅到的事件，还可以约束订阅事件在序列中的位置。比如我们像订阅非Task3而且对应的序列索引小于1：

```swift
tasks.takeWhileWithIndex{ taskn, index in
    taskn != "task3" && index < 1
    }
    .subscribe {
        print($0)
    }
    .disposed(by: bag)
```

不过在最新版的RxSwift中已经遗弃了`takeWhileWithIndex`

```swift
tasks.enumerated().takeWhile { index, element in
    element != "task3" && index < 1
    }.map {
        return $1
    }
    .subscribe {
        print($0)
}
```

上面两部分代码的效果都一样：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_takewhile_index.png)

> **上面无论takeWhile还是takeWhileIndex在closure里写的，是读取事件的条件，而不是终止读取的条件**。



#### takeUntil

和跳过事件的skipUntil对应。我们也可以依赖另外一个外部事件，表达“直到某件事件发生前，一直订阅”这样的语义。例如下面的例子：

```swift
example("takeUntil") {
    let tasks = PublishSubject<String>()
    let bag = DisposeBag()
    let bigBossHasGone =  PublishSubject<String>()
    tasks.takeUntil(bigBossHasGone)
            .subscribe {
                print($0)
            }
            .disposed(by: bag)
    tasks.onNext("Task1")
    tasks.onNext("Task2")
    tasks.onNext("Task3")
    bigBossHasGone.onNext("big boss has gone")
    tasks.onNext("Task4")
    tasks.onCompleted()
}
```

我们在Task4前面发送一条`bigBossHasGone.onNext("big boss has gone")`消息，查看结果：

![有帮助的截图]({{ site.url }}/assets/postsImages/rxswift_choosing_takewhile_takeUntil.png)

可以看到Task1，Task2，Task3都订阅了，当发现bigBossHasGone这个事件发送的时候，就不再订阅事件，所以Task4也就没有订阅到。

#### 最后

Filter operator 就先告一段落了，下一篇转向RxSwift的Transform operators。

