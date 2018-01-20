---
layout: post
title: appearance的坑
subtitle: 
author: JackLin
date: 2017-01-19 14:38:55 +0800
---

#####知识点

>
>appearance:   
1.实现了UIAppearance协议的类都可以调用该方法。  
2.只有被UI_APPEARANCE_SELECT宏修饰的属性才能设置，否则会引起异常.  
3.apperance只能在控件显示前设置好，才会有作用

在做iOS短信适配的时候，设置UITabBarController的UITabBarItem属性时，在UITabBarController中使用[UITabBarItem appearace]来设置全局的UITabBarItem属性，后面发现在另外一个地方UITabBarItem显示异常。应该使用获取某个类中类中UITabBarItem，[UITabBarItem appearanceWhenContainedIn:self, nil];