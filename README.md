# 基于跳表实现的KV存储引擎

## 1. 项目说明

+ 项目来源：参考自[基于跳表实现的轻量级键值数据库](https://github.com/youngyangyang04/Skiplist-CPP)，使用C++实现。在随机写读情况下，每秒可处理写请求数（QPS）: 24.39w，每秒可处理读请求数（QPS）: 18.41w

+ 提供接口：
    + insert\_element: 插入数据
    + search\_element: 查询数据
    + delete\_element: 删除数据
    + clear: 清空跳表（新增）
    + display: 打印跳表（修改）
    + load\_data: 加载数据（修改）
    + dump\_data: 数据存盘（修改）
    + size: 返回数据规模
    + empty: 判断跳表是否为空（新增）
         
*补充说明：除了上述接口中对原项目的扩充，main.cpp和stress\_test.cpp也做了一定修改。*

## 2. 测试方式

+ 接口测试--main.cpp

```shell
g++ main.cpp -o main -std=c++11 -pthread
./main
```

+ 压力测试--stress\_test.cpp

```shell
g++ stress_test.cpp -o stress_test -std=c++11 -pthread -D NDEBUG
./stress_test 100000 1
```

## 3. 压测结果

+ 插入操作

<table>
<tr>
<th>线程数</th><th style="text-align:center" colspan="3">1</th><th style="text-align:center" colspan="3">10</th>
</tr>
<tr>
<th>数据量</th><th style="text-align:center">10w</th><th style="text-align:center">50w</th><th style="text-align:center">100w</th><th style="text-align:center">10w</th><th style="text-align:center">50w</th><th style="text-align:center">100w</th>
</tr>
<tr>
<th>耗时(s)</th><th>0.0497647</th><th>0.316646</th><th>0.900972</th><th>0.169021</th><th>0.8071</th><th>1.89245</th>
</tr>
</table>

+ 查询操作

<table>
<tr>
<th>线程数</th><th style="text-align:center" colspan="3">1</th><th style="text-align:center" colspan="3">10</th>
</tr>
<tr>
<th>数据量</th><th style="text-align:center">10w</th><th style="text-align:center">50w</th><th style="text-align:center">100w</th><th style="text-align:center">10w</th><th style="text-align:center">50w</th><th style="text-align:center">100w</th>
</tr>
<tr>
<th>耗时(s)</th><th>0.034439</th><th>0.367128</th><th>1.13072</th><th>0.0103167</th><th>0.0760128</th><th>0.205313</th>
</tr>
</table>
