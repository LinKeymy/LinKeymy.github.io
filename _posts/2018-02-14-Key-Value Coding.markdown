---
layout: post
title: Key-Value Coding
subtitle: 
author: JackLin
date: 2018-02-14 18:32:22 +0800
---

### 前言

Key-Value Coding简称KVC，俗称键值编码。KVC提供了一套不通过访问器(setter和getter)方法或者属性变量，通过Key或者KeyPath直接访问对象属性的机制。访问器也是一种间接访问对象属性的方法,也是我们经常使用的方法，只不过在有些场合更加适合使用KVC。虽然KVC在业务逻辑中很少会使用，但它是Key-Value Observing，Core Data, Cocoa bindings这些模式或者高级特性的基础，因此KVC还是蛮重要的。这里会介绍和KVC相关的术语，然后再看看KVC的具体使用和基本原理。

### 相关术语

|   术语    | 说明                                                         |
| :-------: | :----------------------------------------------------------- |
| Attribute | 它是一个简单属性，是描述某个事物的一个方面，不和其他对象产生任何联系。 |
|    Key    | Key是标识对象具体属性的字符串，相当于对象的访问器名称或者变量名称，不能包含空格。 当然可以自定义，它与具体KVC的访问方法的实现或者实例变量有关 |
|  KeyPath  | KeyPath是指定对象一系列属性，且用.分割每个属性的字符串。字符串序列中的每个key标识前面对象的属性。比如说people.address.street能够获取people的address属性，然后获取到address的street属性。 |

看看代码：这里有一个Person类,有3个属性address属性、name属性和age属性。`address`属性对应的key为字符串`"address"`,`name`属性对应的key为字符串`“name”`,注意这里adderss是私有的属性，表示Person类都不想对外公开自己的地址信息。

```objective-c
@interface Person : NSObject
    
@property (nonatomic, copy) NSString *name;
@property (nonatomic, assign) int age;

@end
    
@interface Person()
    
@property (nonatomic, strong) Address *address;

@end

```

然后`Address`类中包含1个属性`city`，同理对应的key为`“city”`字符串。

```objective-c
@interface Address : NSObject

@property (nonatomic, copy) City *city;

@end
```

最后定义City类中包含两个属性：`name`和`population`他们对应的key为字符串`"name"`、`"population"`。

```objective-c
@interface City : NSObject

@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) int population;

@end

```

那么上面的`keypath`有什么那些呢：

`"address.city"`

`"address.city.name"`和`"address.city.population"`

### KVC的基本使用

我们首先在Person的实现类添加来那个方法

```objective-c
+ (instancetype)alin {
    City *city = [City new];
    city.name = @"深圳";
    city.population = 10000;
    
    Address *address = [Address new];
    address.city = city;
    
    return [Person initWith:address name:@"林荣安" age:25];
}

+ (instancetype)initWith:(Address *)address name:(NSString *)name age:(int)age {
    Person *p = [Person new];
    p.address = address;
    p.name = name;
    p.age = age;
    return p;
}
```

其中`+(instancetype)alin`用来快速创建一个默认的Person对象。`+(instancetype)initWith:(Address *)address name:(NSString *)name age:(int)age`只是一个内部的快速创建方法。随后我们在main中使用他们。

创建一个Person对象alin

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
    }
    return 0;
}
```

通常我们都是使用点语法来访问属性，当然也可以使用这里要介绍的KVC。比如下面打印name和age

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSLog(@"点语法:name:%@,age:%i",alin.name,alin.age);
        NSString *name = [alin valueForKey:@"name"];
        int age = [[alin valueForKey:@"age"] intValue];
        NSLog(@"KVC:name:%@,age:%i",name,age);
    }
    return 0;
}
```

可以看到得到的结果是一样的：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_person_name_age.png)

但是如果这里想打印地址相关的信息的话，点语法就无能为力了，不过可以使用KVC:

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSLog(@"点语法:name:%@,age:%i",alin.name,alin.age);
        NSString *name = [alin valueForKey:@"name"];
        int age = [[alin valueForKey:@"age"] intValue];
        NSLog(@"KVC:name:%@,age:%i",name,age);

        Address *address = [alin valueForKey:@"address"];
        NSLog(@"KVC:city-name:%@",address.city.name);
    }
    return 0;
}
```

上面需要注意的是对于基本的数据类型使用KVC的话得到的是NSNumber类型。我们看看运行结果：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_city_name.png)

其次我们还可以使用keypath直接访问city的name属性:

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSString *cityName = [alin valueForKeyPath:@"address.city.name"];
        NSLog(@"KVC-KeyPath:city-name:%@",cityName);
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_name_keypath.png)

我们还可以使用KVC对属性赋值：

对默认的alin对象，我们将它的名字`name`属性值改为`alin`，地址更改为`上海`。

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        [alin setValue:@"alin" forKey:@"name"];
        [alin setValue:@"上海" forKeyPath:@"address.city.name"];
        NSString *cityName = [alin valueForKeyPath:@"address.city.name"];
        NSLog(@"KVC:  %@明年要去%@",alin.name,cityName);
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_alin_set.png)

上面在使用KVC的`setValue:forKey:` `valueForKeyPath:`等方法的时候，传入的key和keypath都有对应的属性明与之一一对应。如果传入的是错误的key或者keypath回发生什么呢？

### KVC的异常处理

将上面的代码更改为如下。使用`badname`进行作为key对alin进行赋值：

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        [alin setValue:@"alin" forKey:@"badname"];
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_badname.png)

对应`valueForKeyPath:`还是`setValue:forKeyPath:`也是会一样

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        [alin setValue:@"上海" forKeyPath:@"address.city.badname"];
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_keypath_city_badname.png)

其实无论是`valueForKey:`、`setValue:forKey:`，`valueForKeyPath:`还是`setValue:forKeyPath:`如果没有属性名者key对应都会抛出这样的异常，这是因为:

1. 方法`valueForKey:`寻找不到指定Key或者KeyPath匹配的方法或变量名称会自动调用`valueForUndefinedKey:` 抛出`NSUndefinedKeyException`异常 
2. 方法`setValue:forKey:`寻找不到指定Key或者KeyPath匹配的方法或变量名称会自动调用`setValue:forUndefinedKey:` 抛出`NSUndefinedKeyException`异常

**如果想在出现UndefinedKey的情况下不抛出异常的话，需要自己去实现对应类的连个方法：**

**1. setValue:forUndefinedKey:方法**

**2. valueForUndefinedKey:方法**

比如，我们在City的实现中添加如下代码:

```objective-c
@implementation City

- (id)valueForUndefinedKey:(NSString *)key {
    return [NSString stringWithFormat:@"undefinedKey:%@",key];
}

@end
```

上面重写了City的`valueForUndefinedKey:`然后我们测试一下在City中出现UndefinedKey的情况:

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSString *cityName = [alin valueForKeyPath:@"address.city.badname"];
        NSLog(@"address.city.badname:%@",cityName)
    }
    return 0;
}
```

运行看看:

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_override_undefinedKey.png)

明显，没有抛出异常，仅仅是打印了在我们返回的字符串而已。到这里，对应KVC的基本使用算是介绍的出不多了，或许你和我一样会对KVC的原理有所好奇。

### KVC的基本原理

从上面的实践可以看到，KVC的方法从功能上分存、取两种方法`setValue:forKey:`和`valueForKey:`，以这两个方法为代表描述执行过程。

#### **setValue:forKey:的执行过程** 

1. 首先对象方法列表中匹配方法`-set<Key>:`
2. 如果第1步失败而且 `accessInstanceVariablesDirectly` 返回YES,按照以下顺序匹配实例变量`_<key>, _is<Key>, <key>, or is<Key>`
3. 如果前2步任一成功，则进行赋值。必要的话进行数据类型转换。
4. 如果前3步进行失败则调用 `setValue:forUndefinedKey:` 抛出`NSUndefinedKeyException`异常。

> 方法setValue:forKey:根据指定路径获取属性值，KeyPath中每一个key都进行以上步骤；也就是说任何一个key出错，都会抛出异常。

#### **valueForKey:执行过程**

1. 首先按照此顺序匹配方法 `get<Key>, <key>, or is<Key>,` 如果匹配成功调用方法，返回结果。必要的话进行数据类型转换。
2. 如果1步进行失败，则匹配以下方法 `countOf<Key>、 objectIn<Key>AtIndex: 、 <key>AtIndexes:`若找打其中一个，则返回容器类对象。该对象调用以上方法，会调用valueForKey:方法。(NSArray类的方法)
3. 如果前2步失败，则匹配以下方法`countOf<Key>, enumeratorOf<Key>, and memberOf<Key>:`若找打其中一个，则返回容器类对象。该对象调用以上方法，会调用valueForKey:方法。 (NSSet类的方法)
4. 如果前3步失败，而且 `accessInstanceVariablesDirectly` 返回YES，按照以下顺序匹配实例变量`_<key>, _is<Key>, <key>, or is<Key>`。如果实例变量找到了，则进行复制。必要的话进行数据类型转换。
5. 如果前4步进行失败则调用 `valueForUndefinedKey:` 抛出`NSUndefinedKeyException`异常。

> 1. 方法valueForKeyPath:根据指定路径获取属性值，KeyPath中每一个key都进行以上步骤；也就是说任何一个key出错，都会抛出异常。 
> 2. 如果KeyPath序列中包含了一个key是一对多的关系，而且这个key不是最后一个，那么将返回所有对象的属性值。例如accounts.transactions.payee将返回所有account的所有transaction的所有payee值。

### 非对象类型的处理

KVC对于基本数据类型和结构体在底层支持自动数据类型转换。根据相对的存取方法或者实例变量判端实际需要的值类型，选择NSNumber 或 NSValue 进行自动转换。 

#### NSNumber对应的基本数据类型 

下面是从Apple的文档中截取的基本数据类型和NSNumber转换对应的table：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_nsvalue.png)

例如，Person类添加多一个属性isNice表示是不是友好的人：

```objective-c
+ (instancetype)alin;

@property (nonatomic, copy) NSString *name;
@property (nonatomic, assign) int age;
@property (nonatomic, assign) Bool isNice

@end
```

在main.m中使用setValue:forKey:和valueForKey操作age和isNice:

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSString *cityName = [alin valueForKeyPath:@"address.city.badname"];
        [alin setValue:[NSNumber numberWithBool:YES] forKey:@"isNice"];
        NSLog(@"isNice:%i",[[alin valueForKey:@"isNice"] boolValue])
        NSLog(@"age:%i",[[alin valueForKey:@"age"] intValue])
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_nsnumber_log.png)

其他更多的NSNumber类型这里不一一举例了，有兴趣的可以自己试试。

#### NSValue对应的结构体类型 

结构体类型和NSValue的对应table:

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_nsvalue.png)

这也没什么好说的，和上面类似，就是在使用这类数据的时候记得类型转换就好。就不在过多讨论了。

**值得注意：**

*对非对象类型的属性设置nil空值，底层调用`setNilValueForKey:`，抛出`NSInvalidArgumentException`异常。*

例如：

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSString *cityName = [alin valueForKeyPath:@"address.city.badname"];
        [alin setValue:nil forKey:@"isNice"];
    }
    return 0;
}
```

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_setNilValueforKey.png) 

要想在对结构体和基本数据类型设置nil的时候不抛出异常重写`setNilValueForKey:`方法即可.在Person的实现中添加如下代码：

```objective-c
- (void)setNilValueForKey:(NSString *)key {
    NSLog(@"NilValueForKey:%@",key);
}
```

风平浪静地运行：

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_override_setNilValueforKey.png)



### Key-Value Validation

KVC提供一套API使得属性值生效。使得对象有机会接受值、提供默认值、拒绝新值、抛出错误原因。用到的来两个方法有：

```objective-c
- validateValue:forKey:error:
- validateValue:forKeyPath:error:
```

> 上面的方法KVC不会自动调用，需要手动调用。

**默认实现过程:**

1. 调用`validateValue:forKey:error:` 
2. 在对象的方法列表中匹配`validate<Key>:error:` 
3. 如果找到则执行并返回结果 
4. 如果未找到则返回YES，并赋值 

可以看到我们主要利用上面的第二点对属性值进行限制和其他操作，在Person的实现中增添如下一个方法：

```objective-c
-(BOOL)validateIsNice:(id *)value error:(NSError **)outError {
    BOOL isNice = [*value boolValue];
    if (isNice) {
        [self setValue:*value forKey:@"isNice"];
        return YES;
    } else {
        if (outError != NULL) {
            if (outError != NULL){
                NSDictionary *userInfoDict =
                @{ NSLocalizedDescriptionKey : @"ALin must be nice guy" };
                NSError *error =
                [[NSError alloc] initWithDomain:@"isNice"
                                           code:0
                                       userInfo:userInfoDict];
                *outError = error;
            }
            return NO;
        }
    }
    return YES;
}
```

在main.m中手动使用`validateValue:forKey:error:`这也可以在设置值前进行有效检验：

```objective-c
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        Person *alin = [Person alin];
        NSNumber *n = [NSNumber numberWithBool:NO];
        NSError *error = nil;
        [alin validateValue:&n forKey:@"isNice" error:&error];
        NSLog(@"error: %@",error);
    }
    return 0;
}
```

因为给的值是`NO`不符合Alin的特质，所以发生error:

![有帮助的截图]({{ site.url }}/assets/postsImages/kvc_validateIsNice.png)

Key-Value Validation，在开发中很少会使用，不过了解一下也好。

### 容器类

KVC在容器类中的应用主要包括：NSDictionary、NSArray、NSSet三种。对应容器类也也没有什么特别之处，不过值得一提的是NSDictionary，而且功常用一点：

```objective-c
- (void)setValuesForKeysWithDictionary:(NSDictionary<NSString *, id> *)keyedValues;
- (NSDictionary<NSString *, id> *)dictionaryWithValuesForKeys:(NSArray<NSString *> *)keys;
```

1. **setValuesForKeysWithDictionary:**根据指定dic设置对象属性值。使用dic的key来标识属性，dic的value标识值，底层调用setValue:forKey:进行赋值。如果你的app使用网络数据较少，没有用到第三方的解析，就可以考虑使用这个API，通过一个字典来初始化一个数据模型。只要模型的属性name和指点的key同名对应即可。


2. **dictionaryWithValuesForKeys:**获取一组key的属性值，然后以NSDictionary形式返回。

> **Tips:**
>
> 1. 如果dic中有未定义的key那么需要进行异常处理
> 2. 容器类比如NSArray, NSSet, NSDictionary不能包含nil值，需要使用NSNull替换（一个表示nil值的单例类） 
> 3. 方法dictionaryWithValuesForKeys:和setValuesForKeysWithDictionary:会自动转换NSNull和nil，不需要过多关注

到这相信大家对KVC都有了一个和直观的了解，可以考虑在平时的开发中将KVC用起来。这篇文章有些过长，能看到是需要有信仰。非常感谢！

### The end