---
layout: post
title: CoreAnimation:Core Animation Basics
subtitle: 
author: JackLin
date: 2018-02-28 22:30:00 +0800
---

# Core Animation Basics

Core Animation 提供了一个通用型的机制来为你的app的视图(views)和其他可见的元素做动画。Core Animation并不是用来代替你app中使用的视图。反而，它是一种和视图整合在一起用于提供更好的性能并支持它们的内容做动画的一种技术。它是通过将视图中的内容缓存到可以被图形硬件直接操控的位图中来实现这样的行为(特性、功能);在某些情况，这种缓存行为也许要求你去重新构思如何展现并管理你app的内容，但是大部分时间你使用Core Animation的时候你并不需要知道有缓存行为的存在。除了缓存试图的内容，Core Animation还提供了一种途径去指定任意的可见的内容，将这些内容和视图整合到一起并且对它和它的其他所有内容做动画。你可以使用 Core Animation 去为你app的视图和可见的对象的变动做一些动画。大部分改变都是和你可见对象的属性相关的。比如，你也行使用Core Animation去为你视图的位置、大小、不透明度的改变做一些动画当你修改你的属性时，Core Animation会在当前值和你指定的新的值之间做动画。显然，你也许并不需要像一个卡通片一样使用Core Animation，做到60帧每秒的速率去更换你试图的内容。其实，你通常使用Core Animation用来在屏幕上的移动一个试图的内容、使内容渐进或者渐出、作用与任意的图形变化或者改变其他试图在视觉上的属性。

## Layers Provide the Basis for Drawing and Animations

图层对象是你使用Core Animation去做每一件事情的核心，它是一个被3维空间管理的二维平面。和视图一样，图层管理了和它表面相关的几何、内容和视觉属性信息。和视图不同的是，图层并没有定义它们自己的外表，一个图层仅仅是管理了和位图相关的状态信息。这个位图本身可以是你的视图绘制的自己的内容，或者是你指定的一张图片。基于这个理由，你主要使用的图层可以被看作是一个数据模型，因为它们主要是用来管理数据的。这个概念非常重要，需要记住，因为它影响到做动画的行为。

### The Layer-Based Drawing Model

在你app中，大多数的图层都没有做任何实际的绘图工作。一个图层对象捕获你app提供的内容并将它缓存到位图中，这位图有时也被称为 *backing store*. 随后当你改变这个图层的一个属性，所有你做的实际是在改变和这个图层相关的状态信息。如图1-1，当某个变化促发了一个动画， Core Animation将这个图层的位图和状态信息传递给图形硬件，随后图形硬件使用新的信息将位图提交渲染。在硬件层面操纵位图去做更快更吊的的动画比在软件上实现好多了。

**Figure 1-1**  How Core Animation draws content![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/basics_layer_rendering_2x.png)

由于它是直接操作一个静态的位图，所以基于layer的绘制技术相比更加传统的基于视图的绘制技术有很大的区别。如果是使用基于视图的绘制技术，改变了一个视图本身会导致它调用视图的 `drawRect:`方法使用新的参数去重绘它的内容。这样的方式是非常昂贵的，因为它是在主线程使用CPU来完成这一个工作。Core Animation可以很好的避免这种消耗，它在任何时候都尽可能将位图缓存到硬件中来实现相同或者相似的效果。尽管 Core Animation尽可能地去使用缓存的内容，但是你的app还是必须要时时提供并更新视图的内容。你的app有好几种方式可以给layer提供内容，关于部分更加详细的描述在[Providing a Layer’s Contents](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/SettingUpLayerObjects/SettingUpLayerObjects.html#//apple_ref/doc/uid/TP40004514-CH13-SW4).这里。

### Layer-Based Animations

The data and state information of a layer object is decoupled from the visual presentation of that layer’s content onscreen. This decoupling gives Core Animation a way to interpose itself and animate the change from the old state values to new state values.

比如，改变一个图层的position属性促发Core Animation将移动图层从它当前位置移动到指定的新的位置。相似地，改变其他的属性也会产生相关的动画。图 1-2 展示说明了一小部分你可以在图层中执行动画的类型。这里有一张包含layer可以触发动画的属性表，请看：[Animatable Properties](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/AnimatableProperties/AnimatableProperties.html#//apple_ref/doc/uid/TP40004514-CH11-SW1).

**Figure 1-2**  Examples of animations you can perform on layers![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/basics_animation_types_2x.png)



在做动画的过程中，Core Animation为你一帧一帧地绘制所有的内容。你仅仅需要做的就是指定动画开始和结束的位置，剩下的全部交给Core Animation来做。如果有需要，你还可以指定自定义的时间信息和动画想过的参数。如果你不需要自己定义的话也无妨，Core Animation提供了合适的默认值了。想了解更多关于如何初始化动画和配置动画参数的一下信息，请看： [Animating Layer Content](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/CreatingBasicAnimations/CreatingBasicAnimations.html#//apple_ref/doc/uid/TP40004514-CH3-SW1)

## Layer Objects Define Their Own Geometry

One of the jobs of a layer is to manage the visual geometry for its content. 

图层其中的一个工作是为它的内容管理视觉上的几何结构。这个视觉的几何结构包含了这些信息： 内容的bounds，内容的在屏幕上的位置和图层是否以任一一种方式被旋转，缩放，变换。和视图类似，一个layer有一个矩形的frame和bounds，你可以用来定位你的layer和它的内容。

Layers also have other properties that views do not have, such as an anchor point, which defines the point around which manipulations occur. 

layers 也有一些属性是view没有的，比如 anchor point。它是定义了一个控制的点(参考点？)。你设置图层的一些几何结构的外表的方式也和你设置view的一些信息的方式不同。



>  打算在接下来的几篇文章去尝试翻译Apple的Core Animation Programming Guid，但是最近面试较忙，这个计划得缓缓了，先将英文版的内容贴一些到这里回头有时间再翻译编辑。

### Layers Use Two Types of Coordinate Systems

Layers make use of both *point-based coordinate systems* and *unit coordinate systems* to specify the placement of content. Which coordinate system is used depends on the type of information being conveyed. Point-based coordinates are used when specifying values that map directly to screen coordinates or must be specified relative to another layer, such as for the layer’s `position` property. Unit coordinates are used when the value should not be tied to screen coordinates because it is relative to some other value. For example, the layer’s `anchorPoint` property specifies a point relative to the bounds of the layer itself, which can change.

Among the most common uses for point-based coordinates is to specify the size and position of the layer, which you do using the layer’s `bounds` and `position` properties. The `bounds` defines the coordinate system of the layer itself and encompasses the layer’s size on the screen. The `position` property defines the location of the layer relative to its parent’s coordinate system. Although layers have a `frame` property, that property is actually derived from the values in the `bounds` and `position` properties and is used less frequently.

The orientation of a layer’s `bounds` and `frame` rectangles always matches the default orientation of the underlying platform. Figure 1-3 shows the default orientations of the bounds rectangle on both iOS and OS X. In iOS, the origin of the bounds rectangle is in the top-left corner of the layer by default, and in OS X it is in the bottom-left corner. If you share Core Animation code between iOS and OS X versions of your app, you must take such differences into consideration.

**Figure 1-3**  The default layer geometries for iOS and OS X![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/layer_coords_bounds_2x.png)

One thing to note in [Figure 1-3](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/CoreAnimationBasics/CoreAnimationBasics.html#//apple_ref/doc/uid/TP40004514-CH2-SW2) is that the `position` property is located in the middle of the layer. That property is one of several whose definition changes based on the value in the layer’s `anchorPoint` property. The anchor point represents the point from which certain coordinates originate and is described in more detail in [Anchor Points Affect Geometric Manipulations](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/CoreAnimationBasics/CoreAnimationBasics.html#//apple_ref/doc/uid/TP40004514-CH2-SW17).

The anchor point is one of several properties that you specify using the unit coordinate system. Core Animation uses unit coordinates to represent properties whose values might change when the layer’s size changes. You can think of the unit coordinates as specifying a percentage of the total possible value. Every coordinate in the unit coordinate space has a range of `0.0` to `1.0`. For example, along the x-axis, the left edge is at the coordinate `0.0` and the right edge is at the coordinate `1.0`. Along the y-axis, the orientation of unit coordinate values changes depending on the platform, as shown in Figure 1-4.

**Figure 1-4**  The default unit coordinate systems for iOS and OS X![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/layer_coords_unit_2x.png)

**Note:** Until OS X 10.8, the `geometryFlipped` property was a way to change the default orientation of a layer’s y-axis when needed. Use of this property was sometimes necessary to correct the orientation of a layer when flip transforms were in involved. For example, if a parent view used a flip transform, the contents of its child views (and their corresponding layers) would often be inverted. In such cases, setting the `geometryFlipped` property of the child layers to `YES` was an easy way to correct the problem. In OS X 10.8 and later, AppKit manages this property for you and you should not modify it. For iOS apps, it is recommended that you do not use the `geometryFlipped` property at all.

All coordinate values, whether they are points or unit coordinates are specified as floating-point numbers. The use of floating-point numbers allows you to specify precise locations that might fall between normal coordinate values. The use of floating-point values is convenient, especially during printing or when drawing to a Retina display where one point might be represented by multiple pixels. Floating-point values allow you to ignore the underlying device resolution and just specify values at the precision you need.

### Anchor Points Affect Geometric Manipulations

Geometry related manipulations of a layer occur relative to that layer’s anchor point, which you can access using the layer’s `anchorPoint` property. The impact of the anchor point is most noticeable when manipulating the `position` or `transform` properties of the layer. The position property is always specified relative to the layer’s anchor point, and any transformations you apply to the layer occur relative to the anchor point as well.

Figure 1-5 demonstrates how changing the anchor point from its default value to a different value affects the `position`property of a layer. Even though the layer has not moved within its parents’ bounds, moving the anchor point from the center of the layer to the layer’s bounds origin changes the value in the `position` property.

**Figure 1-5**  How the anchor point affects the layer’s position property![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/layer_coords_anchorpoint_position_2x.png)

Figure 1-6 shows how changing the anchor point affects transforms applied to the layer. When you apply a rotation transform to the layer, the rotations occur around the anchor point. Because the anchor point is set to the middle of the layer by default, this normally creates the kind of rotation behavior that you would expect. However, if you change the anchor point, the results of the rotation are different.

**Figure 1-6**  How the anchor point affects layer transformations![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/layer_coords_anchorpoint_transform_2x.png)

### Layers Can Be Manipulated in Three Dimensions

Every layer has two transform matrices that you can use to manipulate the layer and its contents. The `transform`property of `CALayer` specifies the transforms that you want to apply both to the layer and its embedded sublayers. Normally you use this property when you want to modify the layer itself. For example, you might use that property to scale or rotate the layer or change its position temporarily. The `sublayerTransform` property defines additional transformations that apply only to the sublayers and is used most commonly to add a perspective visual effect to the contents of a scene.

Transforms work by multiplying coordinate values through a matrix of numbers to get new coordinates that represent the transformed versions of the original points. Because Core Animation values can be specified in three dimensions, each coordinate point has four values that must be multiplied through a four-by-four matrix, as shown in Figure 1-7. In Core Animation, the transform in the figure is represented by the `CATransform3D` type. Fortunately, you do not have to modify the fields of this structure directly to perform standard transformations. Core Animation provides a comprehensive set of functions for creating scale, translation, and rotation matrices and for doing matrix comparisons. In addition to manipulating transforms using functions, Core Animation extends key-value coding support to allow you to modify a transform using key paths. For a list of key paths you can modify, see [CATransform3D Key Paths](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Key-ValueCodingExtensions/Key-ValueCodingExtensions.html#//apple_ref/doc/uid/TP40004514-CH12-SW1).

**Figure 1-7**  Converting a coordinate using matrix math![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/transform_basic_math_2x.png)

Figure 1-8 shows the matrix configurations for some of the more common transformations you can make. Multiplying any coordinate by the identity transform returns the exact same coordinate. For other transformations, how the coordinate is modified depends entirely on which matrix components you change. For example, to translate along the x-axis only, you would supply a nonzero value for the `tx` component of the translation matrix and leave the `ty` and `tz`values to 0. For rotations, you would provide the appropriate sine and cosine values of the target rotation angle.

**Figure 1-8**  Matrix configurations for common transformations![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/transform_manipulations_2x.png)

For information about the functions you use to create and manipulate transforms, see *Core Animation Function Reference*.

## Layer Trees Reflect Different Aspects of the Animation State

An app using Core Animation has three sets of layer objects. Each set of layer objects has a different role in making the content of your app appear onscreen:

- Objects in the *model layer tree* (or simply “layer tree”) are the ones your app interacts with the most. The objects in this tree are the model objects that store the target values for any animations. Whenever you change the property of a layer, you use one of these objects.
- Objects in the *presentation tree* contain the in-flight values for any running animations. Whereas the layer tree objects contain the target values for an animation, the objects in the presentation tree reflect the current values as they appear onscreen. You should never modify the objects in this tree. Instead, you use these objects to read current animation values, perhaps to create a new animation starting at those values.
- Objects in the *render tree* perform the actual animations and are private to Core Animation.

Each set of layer objects is organized into a hierarchical structure like the views in your app. In fact, for an app that enables layers for all of its views, the initial structure of each tree matches the structure of the view hierarchy exactly. However, an app can add additional layer objects—that is, layers not associated with a view—into the layer hierarchy as needed. You might do this in situations to optimize your app’s performance for content that does not require all the overhead of a view. Figure 1-9 shows the breakdown of layers found in a simple iOS app. The window in the example contains a content view, which itself contains a button view and two standalone layer objects. Each view has a corresponding layer object that forms part of the layer hierarchy.

**Figure 1-9**  Layers associated with a window![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/sublayer_hierarchy_2x.png)

For every object in the layer tree, there is a matching object in the presentation and render trees, as shown in Figure 1-10. As was previously mentioned, apps primarily work with objects in the layer tree but may at times access objects in the presentation tree. Specifically, accessing the `presentationLayer` property of an object in the layer tree returns the corresponding object in the presentation tree. You might want to access that object to read the current value of a property that is in the middle of an animation.

**Figure 1-10**  The layer trees for a window![img](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/Art/sublayer_hierarchies_2x.png)

**Important:** You should access objects in the presentation tree only while an animation is in flight. While an animation is in progress, the presentation tree contains the layer values as they appear onscreen at that instant. This behavior differs from the layer tree, which always reflects the last value set by your code and is equivalent to the final state of the animation.

## The Relationship Between Layers and Views

Layers are not a replacement for your app’s views—that is, you cannot create a visual interface based solely on layer objects.



 Layers provide infrastructure for your views. Specifically, layers make it easier and more efficient to draw and animate the contents of views and maintain high frame rates while doing so. However, there are many things that layers do not do. Layers do not handle events, draw content, participate in the responder chain, or do many other things. For this reason, every app must still have one or more views to handle those kinds of interactions.

In iOS, every view is backed by a corresponding layer object but in OS X you must decide which views should have layers. In OS X v10.8 and later, it probably makes sense to add layers to all of your views. However, you are not required to do so and can still disable layers in cases where the overhead is unwarranted and unneeded. Layers do increase your app’s memory overhead somewhat but their benefits often outweigh the disadvantage, so it is always best to test the performance of your app before disabling layer support.

When you enable layer support for a view, you create what is referred to as a *layer-backed view*. In a layer-backed view, the system is responsible for creating the underlying layer object and for keeping that layer in sync with the view. All iOS views are layer-backed and most views in OS X are as well. However, in OS X, you can also create a *layer-hosting view*, which is a view where you supply the layer object yourself. For a layer-hosting view, AppKit takes a hands off approach with managing the layer and does not modify it in response to view changes.

**Note:** For layer-backed views, it is recommended that you manipulate the view, rather than its layer, whenever possible. In iOS, views are just a thin wrapper around layer objects, so any manipulations you make to the layer usually work just fine. But there are cases in both iOS and OS X where manipulating the layer instead of the view might not yield the desired results. Wherever possible, this document points out those pitfalls and tries to provide ways to help you work around them.

In addition to the layers associated with your views, you can also create layer objects that do not have a corresponding view. You can embed these standalone layer objects inside of any other layer object in your app, including those that are associated with a view. You typically use standalone layer objects as part of a specific optimization path. For example, if you wanted to use the same image in multiple places, you could load the image once and associate it with multiple standalone layer objects and add those objects to the layer tree. Each layer then refers to the source image rather than trying to create its own copy of that image in memory.

For information about how to enable layer support for your app’s views, see [Enabling Core Animation Support in Your App](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/SettingUpLayerObjects/SettingUpLayerObjects.html#//apple_ref/doc/uid/TP40004514-CH13-SW5). For information on how to create a layer object hierarchy, and for tips on when you might do so, see [Building a Layer Hierarchy](https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CoreAnimation_guide/BuildingaLayerHierarchy/BuildingaLayerHierarchy.html#//apple_ref/doc/uid/TP40004514-CH6-SW2).

