# logging 异步IO日志 #
----------
logging是一个仿python日志系统风格的C++日志模块库，支持向多种流输出日志（暂时只支持标准输出与文件输出，其他输出流待拓展），具有效率高、简单易用、拓展性强的特点。

## 工作原理 ##
+ 缓冲区由若干`Cell`单元组成循环链表，在`Records`类中管理。日志缓存在`Cell`中，自身带有标示指示是否可继续写入。
+ `Records`中两个指针表示生产者指针与消费者指针，指向循环链表中的两个`Cell`。

生产者工作原理：

1. 生成格式化日志文本；
2. 上锁；
3. 检查：当前日志输出流与当前生产者指针指向的`Cell`是否相同以及是否有足够空间写入日志；
4. 若以上结果为`false`，将当前`Cell`标记为已满，检查下一格`Cell`是否为空；
5. 若下一格为空，移动生产者指针到下一格`Cell`，否则申请往下一格插入新`Cell`，若申请成功，移动生产者指针，否则说明缓冲区已满，直接丢弃本次日志；
6. 将本次日志写入生产者指针指向的`Cell`；
7. 如本次操作曾将`Cell`标记为已满，则唤醒消费者。

消费者工作原理：

消费者作为后台线程将一直保持循环，直到程序结束。

1. 等待生产者唤醒；
2. 循环：检查当前消费者指针指向的`Cell`是否已满；若已满，将当前`Cell`中的日志写入对应输出流，将`Cell`标记为空，并移动消费者指针到下一格；否则返回步骤1。

优化：

1、优化格式化时间的生成：当秒数间隔无需进位时，通过计算只刷新秒数，减少系统调用次数，提高性能；

2、调整缓存单元的数量与大小，减少新单元插入次数，提高性能。

## 日志格式 ##
YYYY-MM-DD hh:mm:ss [LEVEL] RelativeCreatedTime [SOURCE]: MESSAGE

## 编译 ##

makefile文件包含编译静态库与动态库。

## 使用方法 ##

1. 直接输出至标准输出：

注意：程序结束前请调用`logging::flush`函数，确保缓冲区的数据被全部刷写。

```C++
#include "logging.hpp" //下略

int main()
{
	logging::info("This is a test.");
	logging::debug("This is another test.", "NamedSource");
	logging::flush();	//等待刷写缓冲区
	return 0;
}
```

标准输出：
```
2020-07-28 17:11:16 [INFO] 0.01 [default]: This is a test.
2020-07-28 17:11:16 [DEBUG] 0.01 [NamedSource]: This is another test.
```

2. 初始化指定默认来源并输出至标准输出：

初始化logging时可指定日志的默认来源、默认等级以及开头说明（可选参数）。

```C++
int main()
{
	logging::config("tester", Logging::DEBUG, "This is a header."); //注意等级处为Logging
	logging::info("This is a test.");
	logging::debug("This is another test.", "source");
	logging::flush();
	return 0;
}
```

标准输出：
```
Log for [tester] created at: 2020-07-28 17:13:01
This is a header.
2020-07-28 17:13:01 [INFO] 0.01 [tester]: This is a test.
2020-07-28 17:13:01 [DEBUG] 0.01 [source]: This is another test.
```


3. 输出至文件及其他输出流：

创建输出流并在初始化logging时指定输出流，若无指定输出流将输出至标准输出。

```C++
int main()
{
	Logging::fileHandler file("test.log");
	logging::config("tester", Logging::DEBUG, "This is a header.", &file);
	logging::info("This is a test.");
	logging::debug("This is another test.", "source");
	logging::flush();
	return 0;
}
```

test.log内容:
Log for [tester] created at: 2020-07-28 17:14:04
This is a header.
2020-07-28 17:14:04 [INFO] 0.01 [tester]: This is a test.
2020-07-28 17:14:04 [DEBUG] 0.01 [source]: This is another test.

4. 输出至多个输出流：

```C++
int main()
{
	Logging::fileHandler file1("test1.log");
	Logging::fileHandler file2("test2.log");
	size_t log_id1 = logging::config("tester1", Logging::DEBUG, "This is a header.", &file1);
	size_t log_id2 = logging::config("tester2", Logging::DEBUG, "This is a header.", &file2);
	logging::info("This is a test.", log_id1);
	logging::debug("This is another test.", "source", log_id2);
	logging::flush();
	return 0;
}
```

test1.log内容：
Log for [tester1] created at: 2020-07-28 17:15:08
This is a header.
2020-07-28 17:15:08 [INFO] 0.01 [tester1]: This is a test.

test2.log内容：
Log for [tester2] created at: 2020-07-28 17:15:08
This is a header.
2020-07-28 17:15:08 [DEBUG] 0.01 [source]: This is another test.

## 性能测试 ##

1. 单线程连续写入1亿条日志；
2. 10线程各自连续写入500万条日志。

| 写入方式 | 第一次 | 第二次 | 第三次 | 第四次 | 第五次 | 平均 | 速度 |
| :----: | :----: | :----: | :----: | :----: | :----: | :----: | :----: |
| 单线程 | 77.81 | 76.86 | 77.89 | 77.49 | 77.85 | 77.58 | 128.9万/s |
| 多线程 | 47.09 | 46.30 | 48.55 | 46.36 | 47.05 | 47.07 | 106万/s |

## To Do ##

+ 优化多输出流情况下日志在`Cell`中的储存方式；
+ 更多单元测试；
+ 程序异常退出时缓冲区丢失问题；
+ 进一步优化`Cell`的体积与数量。