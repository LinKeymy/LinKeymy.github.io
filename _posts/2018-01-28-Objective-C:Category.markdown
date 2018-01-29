---
layout: post
title: Objective-C:Category
subtitle: 
author: JackLin
date: 2018-01-29 00:27:23 +0800

---



```oc
struct category_t {
    const char *name;
    classref_t cls;
    struct method_list_t *instanceMethods;
    struct method_list_t *classMethods;
    struct protocol_list_t *protocols;
    struct property_list_t *instanceProperties;
};
```

