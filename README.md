# 基于跳表实现的STL-style关联容器(set/map)

## 1. 项目说明

+ 项目简介：跳表作为许多非关系型数据库的核心存储引擎具有极佳的效率，而STL中的关联容器使用的底层数据结构为红黑树和哈希表。因此本项目以跳表为基础，并参考SGI STL的风格，使用C++实现了set和map这两种关联容器，除了常规的增删改查操作外，还实现了跳表专属迭代器，以兼容STL的通用算法，例如：

```cpp
// 通过一对迭代器范围内的数据来构造iset
std::vector<int> ivec = {7, 0, 3, 1, 4, 9, 2};
skip_set<int> iset(ivec.begin(), ivec.end());

// 通过迭代器和STL通用的copy函数将iset中的所有元素写入输出流
std::ostream_iterator<int> oiter(std::cout, " ");
copy(iset.begin(), iset.end(), oiter);
```

+ 参考资料：
    + [《STL源码剖析》](https://github.com/tolerious/Programming_learning_resource/blob/master/C%2B%2B/STL%E6%BA%90%E7%A0%81%E5%89%96%E6%9E%90%EF%BC%88%E6%89%B9%E6%B3%A8%E7%89%88%EF%BC%89.pdf)
    + [基于跳表实现的轻量级键值数据库](https://github.com/youngyangyang04/Skiplist-CPP)
    
## 2. 测试方式

+ 接口测试：
    + test\_set.cpp：测试set接口，用例源于《STL源码剖析》第236页
    ```shell
    g++ test_set.cpp -std=c++11 & ./a.out
    ```
    + test\_map.cpp：测试map接口，用例源于《STL源码剖析》第242页
    ```shell
    g++ test_map.cpp -std=c++11 & ./a.out
    ```

+ 压力测试：
    + stress.cpp：测试插入和查找的效率
    ```shell
    g++ stress.cpp -o stress -std=c++11 -D NDEBUG
    
    # 以数据量为10W，线程数为1进行测试
    ./stress 100000 1
    ```

## 3. 压测结果

<table>
<tr>
<th style="text-align:center" colspan="2">数据量</th><th style="text-align:center">1W</th><th style="text-align:center">5W</th><th sytle="text-align:center">10W</th>
</tr>
<tr>
<th rowspan="3">耗时(s)</th><th>insert</th><th>0.0142734</th><th>0.0493843</th><th>0.0830898</th>
</tr>
<tr>
<th>find</th><th>0.0094245</th><th>0.0275795</th><th>0.0471889</th>
</tr>
<tr>
<th>std::find</th><th>0.4745585</th><th>13.7145351</th><th>59.6562138</th>
</tr>
</table>

可以看出虽然使用STL通用版的find函数也可以有效运作，但其时间复杂度要远高于使用成员版的find函数，更加凸显了底层跳表结构极佳的查询效率。
