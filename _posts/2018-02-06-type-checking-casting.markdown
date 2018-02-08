---
layout: post
title: Swift:类型检查和类型转换
subtitle: 
author: JackLin
date: 2018-02-06 13:08:35 +0800
---

### 前言

如果你有Objective-C、C的使用经验，你会经常在变量前面使用类型修饰对类型进行强制转换: 

```objective-c
id str = @"alin";
if ([str isMemberOfClass:[NSString class]]) {
    str = (NSString *)str;
}
```

上面把起初声明为id类型的str通过类型修饰强转为NSString类型。在Swift的使用中，我们也无法避免要使用到类型检查和类型转换。那在Swift有时如何处理呢？一起看看吧。

> 关键词：Checking type、Type casting、Downcasting、is、as、as？、as！

### 类型检查

在Objectvie-C通常是使用如下像个方法进行类型检查：

```objective-c
-(BOOL) isKindOfClass:  判断是否是这个类或者这个类的子类的实例
-(BOOL) isMemberOfClass:  判断是否是这个类的实例
```

Swift 中类型检查的实现为 `is`操作符。这个操作符简单传神。它的作用相当于`isKindOfClass:`所以说：使用*类型检查操作符* （ is ）来检查一个实例是否属于一个特定的子类。如果实例是该子类类型，类型检查操作符返回 true ，否则返回 false 。

```swift
class MediaItem {
    var name: String
    init(name: String) {
        self.name = name
    }
}
class Movie: MediaItem {
    var director: String
    init(name: String, director: String) {
        self.director = director
        super.init(name: name)
    }
}
class Song: MediaItem {
    var artist: String
    init(name: String, artist: String) {
        self.artist = artist
        super.init(name: name)
    }
}

let library = [
    Movie(name: "Casablanca", director: "Michael Curtiz"),
    Song(name: "Blue Suede Shoes", artist: "Elvis Presley"),
    Movie(name: "Citizen Kane", director: "Orson Welles"),
    Song(name: "The One And Only", artist: "Chesney Hawkes"),
    Song(name: "Never Gonna Give You Up", artist: "Rick Astley")
]
var movieCount = 0
var songCount = 0
 
for item in library {
    if item is Movie {
        movieCount += 1
    } else if item is Song {
        songCount += 1
    }
}
print("Media library contains \(movieCount) movies and \(songCount) songs")
// Prints "Media library contains 2 movies and 3 songs"
```

如果当前 MediaItem 是 Movie 类型的实例， item is Movie 返回 true ，反之返回 false 。同样的，item is Song 检查了该对象是否为 Song 类型的实例。在 for-in 循环的最后， movieCount 和songCount 的值就是数组中对应类型实例的数量。

如果没有继承关系，那么`is`总是失败。如下:

```swift
for item in library { 
    if item is String {
      // 不会被执行
        print("mediatime")
    }
}
```

### 向下类型转换

个类类型的常量或变量可能实际上在后台引用自一个子类的实例。当你遇到这种情况时你可以尝试使用*类型转换操作符*（ as? 或 as! ）将它*向下类型转换*至其子类类型。

* as!属于强制类型转换，如果转换失败会报 runtime 运行错误。

```swift
class Animal {}
class Cat: Animal {}
let animal :Animal  = Cat()
let cat = animal as! Cat
```

当你确信向下转换类型会成功时，使用强制形式的类型转换操作符（ as! ）。如上面可以确定一定是Cat类型。

* as? 和 as! 操作符的转换规则完全一样。但 as? 如果转换不成功的时候便会返回一个 nil 对象。成功的话返回可选类型值（optional），需要我们拆包使用。由于 as? 在转换失败的时候也不会出现错误，所以对于如果能确保100%会成功的转换则可使用 as!，否则使用 as?

```swift
let animal:Animal = Cat()
 
if let cat = animal as? Cat{
    print("cat is not nil")
} else {
    print("cat is nil")
}
```

> *无论是类型检查还是向下类型转换也好通常都是在继承链上处理的，如果检查判断的类或者转换的类和对象没有任何关系那么编译器会给出总是失败的警告*

![有帮助的截图]({{ site.url }}/assets/postsImages/iswarrring.png)

![有帮助的截图]({{ site.url }}/assets/postsImages/as_warrning.png)



### 向上类型转换、 Any 和 AnyObject 的类型转换和

#### 向上类型转换

向上转换时总是可以有效的(或者说是成功),这时对于向上类型转换都是使用 `as`

```swift
class Animal {}
class Cat: Animal {}
let cat = Cat()
let animal = cat as Animal
```

#### Any 和 AnyObject 的类型转换 

Swift 为不确定的类型提供了两种特殊的类型别名：

- AnyObject  可以表示任何类类型的实例。
- Any  可以表示任何类型，包括函数类型。

AnyObject  和 Any  都不是特定的类型也没有特定的继承链的概念，知识表示一个任意的位置的对象类型或者非对象类型，这时候的类型转换不是从 `特定类型`转为`特定类型`而是从`未知类型`转为`特定类型`。

### 消除二义性，数值类型转换

```swift
let num1 = 42 as CGFloat
let num2 = 42 as Int
let num3 = 42.5 as Int
let num4 = (42 / 2) as Double
```

### Swift和Obejctvie-C的桥接

通常Swift和Obejctvie-C的桥接可以使用`as`操作符。经常碰到的情况可能是在字符串、字典、数组等类型中进行桥接：

```swift
let nsStr: NSString = "NSString"
nsStr.length
let str = nsStr as String
str.count

var nsArray: NSArray = ["str","str"]
let newArray = nsArray.adding("new str")
var array = nsArray as Array
array.append("another str" as NSString)
```



