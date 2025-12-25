# Vector

---

# 1. 介绍

Vector 是一个为 RT-Thread 设计的通用向量容器模块，提供动态大小的数组功能，支持不同数据类型的存储和操作。该模块具有高效的内存管理机制，能够根据实际需求自动调整容量，非常适合需要频繁进行元素添加、删除和访问的应用场景。

## 1.1 主要特性

- 支持任意数据类型的存储（通过 void* 指针和元素大小参数实现）；
- 动态内存管理，自动扩容（容量翻倍）和缩容（当元素数量小于容量一半时）；
- 丰富的操作接口：
  - 元素添加：push_back, push_front, insert
  - 元素删除：pop_back, pop_front, remove
  - 元素访问：get, front, back
  - 元素修改：modify
  - 向量操作：sort, for_each, find
  - 向量管理：clear, shrink, destroy
- 高效的排序算法（归并排序，时间复杂度 O(n log n)）；
- 基于 RT-Thread 内存管理，确保线程安全；
- 支持批量元素操作（push_back_block, insert_block, remove_block）；
- 轻量级设计，占用资源少。

## 1.2 适用场景

- 需要动态调整大小的数组场景；
- 频繁进行元素添加和删除操作；
- 需要支持多种数据类型的通用容器；

# 2. 使用

### 2.1 基本操作流程

Vector 模块的使用遵循创建-操作-销毁的基本流程：

1. 创建向量（vector_create）
2. 执行各种元素操作（添加、删除、修改、访问等）
3. 销毁向量（vector_destroy）

### 2.2 创建向量

```c
vector_config_t config = {
    .item_size = sizeof(int),  // 指定元素大小
    .capacity = 10              // 指定初始容量
};
vector_handle_t v = vector_create(&config);
if (v == RT_NULL) {
    // 创建失败处理
}
```

### 2.3 元素添加

```c
// 尾部添加单个元素
int num = 10;
vector_push_back(v, &num);

// 头部添加单个元素
int num2 = 20;
vector_push_front(v, &num2);

// 指定位置插入元素
int num3 = 15;
size_t index = 1;
vector_insert(v, index, &num3);

// 尾部批量添加元素
int nums[] = {30, 40, 50};
size_t count = sizeof(nums) / sizeof(nums[0]);
vector_push_back_block(v, nums, count);
```

### 2.4 元素删除

```c
// 删除尾部元素
vector_pop_back(v);

// 删除头部元素
vector_pop_front(v);

// 删除指定位置元素
vector_remove(v, index);

// 删除指定范围元素
size_t length = 2;
vector_remove_block(v, index, length);
```

### 2.5 元素访问与修改

```c
// 获取指定位置元素
int *val = (int *)vector_get(v, index);
if (val) {
    rt_kprintf("Element at %d: %d\n", index, *val);
}

// 获取第一个元素
val = (int *)vector_front(v);

// 获取最后一个元素
val = (int *)vector_back(v);

// 修改指定位置元素
int new_val = 100;
vector_modify(v, index, &new_val);
```

### 2.6 向量排序

```c
// 定义比较函数
int int_cmp(void *a, void *b)
{
    int *pa = (int *)a;
    int *pb = (int *)b;
    return (*pa - *pb);
}

// 对向量进行排序
vector_sort(v, int_cmp);
```

### 2.7 向量遍历

```c
// 定义遍历操作函数
void print_element(void *val, size_t index, size_t size, void *context)
{
    if (val) {
        rt_kprintf("Element %d: %d\n", index, *(int *)val);
    }
}

// 遍历向量所有元素
vector_for_each(v, print_element, RT_NULL);
```

### 2.8 元素查找

```c
// 定义查找比较函数
int int_find_cmp(const void *item, const void *data)
{
    int *pitem = (int *)item;
    int *pdata = (int *)data;
    return (*pitem == *pdata) ? 0 : 1;
}

// 查找元素
int target = 10;
uint32_t find_index;
if (vector_find(v, &target, int_find_cmp, &find_index) == 0) {
    rt_kprintf("Found element %d at index %d\n", target, find_index);
}
```

### 2.9 向量管理

```c
// 清空向量所有元素
vector_clear(v);

// 收缩向量内存至实际需要大小
vector_shrink(v);

// 获取向量当前大小
size_t size = vector_size(v);

// 获取向量当前容量
size_t capacity = vector_capacity(v);

// 销毁向量
vector_destroy(v);
```

# 3. Demo

以下是一个简单的 Vector 模块使用示例，展示了基本的创建、添加、遍历和销毁操作：

```c
#include <rtthread.h>
#include "vector.h"

// 遍历回调函数
void print_int(void *val, size_t index, size_t size, void *context)
{
    if (val) {
        rt_kprintf("[%d]: %d\n", index, *(int *)val);
    }
}

// 比较函数
int cmp_int(void *a, void *b)
{
    return (*(int *)a - *(int *)b);
}

int vector_demo(void)
{
    // 创建向量
    vector_config_t config = {
        .item_size = sizeof(int),
        .capacity = 5
    };
    vector_handle_t v = vector_create(&config);
    if (v == RT_NULL) {
        rt_kprintf("Vector create failed!\n");
        return -1;
    }
    rt_kprintf("Vector created successfully!\n");

    // 添加元素
    int nums[] = {5, 2, 8, 1, 9, 3};
    for (size_t i = 0; i < sizeof(nums)/sizeof(nums[0]); i++) {
        vector_push_back(v, &nums[i]);
    }
    rt_kprintf("Added %d elements\n", vector_size(v));

    // 遍历元素
    rt_kprintf("Vector elements before sort:\n");
    vector_for_each(v, print_int, RT_NULL);

    // 排序
    vector_sort(v, cmp_int);
    rt_kprintf("\nVector elements after sort:\n");
    vector_for_each(v, print_int, RT_NULL);

    // 查找元素
    int target = 5;
    uint32_t index;
    if (vector_find(v, &target, cmp_int, &index) == 0) {
        rt_kprintf("\nFound %d at index %d\n", target, index);
    } else {
        rt_kprintf("\nElement %d not found\n", target);
    }

    // 销毁向量
    vector_destroy(v);
    rt_kprintf("Vector destroyed\n");

    return 0;
}

MSH_CMD_EXPORT(vector_demo, Vector module demo);
```

# 4. 文档

详细的 API 说明请参考头文件 `vector.h` 中的注释。


# 6. 许可

MIT Copyright (c) 2020-2025 Gary Zhang [cleancode@163.com]
