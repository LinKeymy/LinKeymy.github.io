---
layout: post
title: JavaScript相关的一些基本内容
subtitle: 
author: JackLin
date: 2018-02-03 16:19:52 +0800
---

### 前言

今天突然想写一篇JavaScript和web相关的内容也当权是温故知新。在开始前就考虑了一下是否需要添加JavaScript语法相关的内容，后来考虑到主题是web相关的内容，而且JavaScript的语法也是和其他语言类似，基本数据类型，字符串操作，集合类型，函数，内置数学函数库支持，对象等，应该语言的大部分概念都是一样的。所以下面的内容可以说是JavaScript在Web中的一下基本应用。

### JS中的window对象

js中内置的一个window对象，它主要有两大作用；

#### 1. 全局变量或者全局函数属于window对象的属性和方法

```javascript
function printName() {
  var name = "onevlin";
  console.log("printName exe");
  console.log(window.name);
}
printName();
window.printName()
```

如上，全局函数可以直接调用，也可以通过window对象调用，但是在printName函数内的局部变量name不属于window对象的属性，也就无法访问了。

#### 2. 页面动态跳转的操作（非a标签实现）

```javascript
window.location.href = 'http://onevlin.com';
```

### JS中的document对象

js中内置的一个document对象，主要是用它来获得网页的内所有的标签（节点）,这样就可以操作对应的标签，对标签进行增删改查等操作了

### JS中常见的事件

- 当页面加载完毕

```js
window.onload = function () {
	console.log("页面加载完毕，类似iOS中的viewDidload")
}
```

- 光标相关事件(类似iOS中touch事件)

```javascript
tag.onmouseover = function () {
	console.log("进入图片")
}

tag.onmousemove = function () {
	console.log("在图片上移动")
}

tag.onmouseout = function () {
	console.log("离开图片")
}
```

- 当输入框获得焦点

```javascript
input.onfocus = function () {
	console.log("框获得焦点")
}
```

- 点击

```javascript
button.onclick = function () {
	console.log("Button.onclick")
}
```

### JS的CRUD

- 增

> 直接写完当前的html文件(加到整一个页面上,类似加到iOS的keyWindow上）

```javascript
document.write('hello world');
```

> 写到某一个标签内 (加到某一个div内，非常像是iOS中动态给view添加一个subview)

```javascript
var mydiv = document.getElementById('mydiv');
var atag = document.creatElement('atag');
atag.href = 'http://www.onevlin.com';
mydiv.addpendChild(mydiv)
```

- 删 

> 删除某个标签等

```javascript
mydiv.remove()  /*类似iOS的removeFromeSuperview*/
```

- 改

> 修改标签的一些属性

```javascript
/*1.先获得标签*/
document.getElement等开头的API
/*2.修改标签的属性值*/
mytag.property = newProperty;
```

- 查

> 查看标签的内容

```javascript
/*1.先获得标签*/
document.getElement等开头的API
/*2.查看标签的属性等内容*/
mytag.childNodes;
......
```

从上面的简单实践来看，直接使用javascript的api进行dom操作十分繁琐。无法直接获得属性，一般操作一个dom都要很麻烦的get。而且在请求网络数据方面还存在跨域的问题。还是第三方大法好！