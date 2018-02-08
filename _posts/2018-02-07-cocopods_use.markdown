---
layout: post
title: cocopods_use
subtitle: 
author: JackLin
date: 2018-02-07 01:22:11 +0800
---





### cocoapods的工作机制

下面是cocoapods简化的工作机制图。



![有帮助的截图]({{ site.url }}/assets/postsImages/cocoapods_summarry.png)



* 1. 如果支持cocoapods方式管理的第三方框架，作者会将他创建的包含框架source地址，版本的信息的.spec文件上传到cocoapods管理的[这个库中](https://github.com/CocoaPods/Specs.git)，如图你会看到很多包含了.spec文件的文件夹。

![有帮助的截图]({{ site.url }}/assets/postsImages/cocoapods_github_specs.png)

* 2. 如果你安装了cocoapods或者执行pod update类似的操作的时候会从上面的库中将master中的所有的Specs文件clone或者pull下了。如果在pod install的时候不指定`pod install --repo-update`的话，就直接根据`Podfile.lock`文件的信息去拉取项目依赖的第三方库。如图：

![有帮助的截图]({{ site.url }}/assets/postsImages/cocoapods_specs_master.jpg)

* 3. 在cocoapods安装完成的时候会在其cache中创建一个方便快速检索的文件`search_index.json`,如果不小心删除了，下次检索还是会先根据客户端的Specs中的文件重新创建。如图：

![有帮助的截图]({{ site.url }}/assets/postsImages/cocoapods_resource_index_json.jpg)

* 4. 同样在我们执行pod install将source拉取下来后，为了下次pod install更加快速会缓存install过的source缓存下来。我电脑上的缓存路径如下图：

![有帮助的截图]({{ site.url }}/assets/postsImages/cocoapods_caches_source.jpg)



### Spec文件的配置









