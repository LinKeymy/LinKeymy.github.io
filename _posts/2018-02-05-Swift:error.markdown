---
layout: post
title: Swift:错误处理
subtitle: 
author: JackLin
date: 2018-02-05 22:09:36 +0800
---

### 前言

本来打算这篇文章仅仅是对Swift的可恢复错误语法的一个简化归纳。后来看到王巍的[关于 Swift Error 的分类](https://onevcat.com/2017/10/swift-error-category/)的这篇文章，改变了主意。这里打算先截取他前面的一部分内容，然后增加错误处理syntax相关的内容。

> 关键词：Swift错误的分类，Swift可恢复错误语法，throw ，Error，fatalError，assert ，try， do-catch

### Swift 错误分类

#### 简单错误

简单的，显而易见的错误。这类错误只需要知道错误发生，并且想要进行处理,一般这里错误都是允许的。用来表示这种错误发生的方法一般就是返回一个 `nil` 值。在 Swift 中，这类错误最常见的情况就是将某个字符串转换为整数，或者在字典尝试用某个不存在的 key 获取元素：

```swift
let num = Int("hello world") // nil
let element = dic["key_not_exist"] // nil
```

在使用层面 (或者说应用逻辑) 上，这类错误一般用 `if let` 的可选值绑定或者是 `guard let` 提前进行返回处理即可，不需要再在语言层面上进行额外处理。

```swift
let num = Int("hello world")
func printReuslt() {
    guard let _ = num else {
        print("error")
       return
    }
    print("nice")
}
```

#### 普遍错误

这类错误理论上可以恢复，但是由于语言本身的特性所决定。这类错误包括类似下面这些情形：

```swift
// 内存不足
[Int](repeating: 100, count: .max)
// 调用栈溢出_死循环
func foo() { foo() }
foo()
```

在 Swift 中，各种被使用 `fatalError` 进行强制终止的错误一般都可以归类普遍错误。

再比如你团队开发中需要提供一个通用的类，但是由于工期有限，有一些功能只定义了接口，没有进行具体实现。这些接口会在提测时完成，但是类中的部分完成的功能需要给团队其他人调用。所以除了在文档中明确标明这些内容，这些方法内部应该如何处理呢？

```swift 
func foo() -> Bar? {
    fatalError("Not implemented yet.")
}
```

> 对于没有实现的方法，返回 `nil` 或者抛出错误时迷惑队友的行为。这里的问题是语言层面的边界情况，由于没有实现，我们需要给出强力的提醒。在任意 build 设定下，都不应该期待队友可以成功调用这个函数，所以 `fatalError` 是最佳选择。

#### 逻辑错误

逻辑错误是程序员的失误所造成的错误，它们应该在开发时通过代码进行修正并完全避免，而不是等到运行时再进行恢复和处理。

```swift
// 强制解包一个 `nil` 可选值
var name: String? = nil
name!

// 数组越界访问
let arr = [1,2,3]
let num = arr[3]

// 计算溢出
var a = Int.max
a += 1

// 强制 try 但是出现错误
try! JSONDecoder().decode(Foo.self, from: Data())
```

这类错误在实现中触发的一般是 [`assert` 或者 `precondition`](https://github.com/apple/swift/blob/a05cd35a7f8e3cc70e0666bc34b5056a543eafd4/stdlib/public/core/Collection.swift#L1009-L1046)。

比如在 app bundle 中需要一张用户默认头像的图片，如果读取时没有找到该图片，我们应该直接断言让程序在开发阶段就发现并处理错误：

```swift
func loadImage() -> UIImage {
    let path = Bundle.main.path(forResource: "my_pre_icon", ofType: "png")!
    let url = URL(fileURLWithPath: path)
    let data = try! Data(contentOf: url)
    return try! ImageLoader.load(from: data)
}
```



#### 可恢复错误

这类错误应该是被容许，并且是可以恢复的。可恢复错误的发生是正常的程序路径之一，而作为开发者，我们应当去检出这类错误发生的情况，并进一步对它们进行处理，让它们恢复到我们期望的程序路径上。比如是网络请求超时，服务器内部错误，参数错误，或者写入文件时磁盘空间不足，格式编码等错误。对于这类运行时可恢复的错误Swift提供类抛出、捕获、传递和操纵等支持。

```swift
// 网络请求
let url = URL(string: "https://www.example.com/")!
let task = URLSession.shared.dataTask(with: url) { data, response, error in
    if let error = error {
        // 提示用户
        self.showErrorAlert("Error: \(error.localizedDescription)")
    }
    let data = data!
    // ...
}

// 写入文件
func write(data: Data, to url: URL) {
    do {
        try data.write(to: url)
    } catch let error as NSError {
        if error.code == NSFileWriteOutOfSpaceError {
            // 尝试通过释放空间自动恢复
            removeUnusedFiles()
            write(data: data, to: url)
        } else {
            // 其他错误，提示用户
            showErrorAlert("Error: \(error.localizedDescription)")
        }
    } catch {
        showErrorAlert("Error: \(error.localizedDescription)")
    }
}
```

### Swift 错误表示、抛出、传递、捕获处理和转化

#### 表示

在 Swift 中，错误表示为遵循 Error协议类型的值。这个空的协议明确了一个类型可以用于错误处理。Swift 枚举是典型的为一组相关错误条件建模的完美配适类型，关联值还允许错误错误通讯携带额外的信息。通常是推荐使用枚举来表示错误，比如说，这是你可能会想到的游戏里自动售货机会遇到的错误条件：

```swift
enum VendingMachineError: Error {
    case invalidSelection
    case insufficientFunds(coinsNeeded: Int)
    case outOfStock
}
```

#### 抛出

在需要抛出错误的地方使用throw来抛出即可：

```swift
throw VendingMachineError.insufficientFunds(coinsNeeded: 5)
```

####  传递

为了明确一个函数或者方法可以抛出错误，你要在它的声明当中的形式参数后边写上 throws关键字。这样这个函数就将内部的错误(传递)抛出到执行函数的地方。因此，抛出错误的函数可以把它内部抛出的错误传递到它被调用的生效范围之内。

```swift
func canThrowErrors() throws -> String
```



> 只有抛出函数可以传递错误。任何在非抛出函数中抛出的错误都必须在该函数内部处理。

#### 使用 do-catch 捕获并处理错误

使用 do-catch语句来通过运行一段代码处理错误。如果do分句中抛出了一个错误，它就会与catch分句匹配，以确定其中之一可以处理错误。

```swift
do {
    try expression
    statements
} catch pattern 1 {
    statements
} catch pattern 2 where condition {
    statements
}
```

如果抛出错误，执行会立即切换到 catch分句，它决定是否传递来继续。如果没有错误抛出， do语句中剩下的语句将会被执行。

#### try？ 将错误转化为可选项

```swift
func someThrowingFunction() throws -> Int {
    // ...
}
let x = try? someThrowingFunction()
```

#### try!将错误转化为运行时断言

```swift
let photo = try! loadImage("./Resources/John Appleseed.jpg")
```



### 最后想说

因为Swift枚举的强大存在，在Swift对可恢复错误的处理也更加方法和清晰。同时Swift的try也不会像Objc中的try那么无人问津。try？和 try！的语法大大方便了我们对错误处理的书写，属于锦上添花吧。个人感觉吧，使用Swift对比Objc更加习惯考虑出现错误的可能性，也更有动力去思考挖掘处理错误。Error还是蛮有魅力的！