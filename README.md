# dmdatetime - 一个现代C++的日期与时间处理库

`dmdatetime` 是一个功能丰富的、仅含头文件的 C++ 库，旨在提供一个类似于 C\# `DateTime` 和 `TimeSpan` 的强大而直观的接口，用于处理日期和时间。它基于标准的 `<ctime>`，并封装了复杂的细节，从而简化了常见的日期时间计算、格式化和解析任务。

## 核心特性

  - **双核心类设计**: `CDMDateTime` 表示一个特定的时间点，`CDMTimeSpan` 表示一个时间间隔，概念清晰。
  - **直观的API**: 提供易于理解和使用的函数，如 `Now()`, `Today()`, `AddDays()`, `GetStartOfMonth()` 等。
  - **强大的算术运算**: 支持年、月、日、时、分、秒的加减，以及时间点和时间段之间的精确运算。
  - **灵活的格式化与解析**:
      - 支持自定义格式的字符串输出 (`ToString`) 和解析 (`Parse`)。
      - 内置了对 ISO 8601 (`ToISOString`, `ToUTCString`) 等标准格式的支持。
      - 预定义了多种常用格式常量。
  - **便捷的工具函数**: 包含检查闰年、工作日/周末、获取月份/年份的开始/结束时间点等大量实用功能。
  - **零依赖**: 仅依赖C++标准库，无需任何第三方库，轻松集成到任何项目中。
  - **跨平台**: 在 Windows 和 Linux 上均表现一致，通过预处理器宏处理了平台特定的函数调用。

## 快速上手

只需将 `dmdatetime.h` 文件包含到您的项目中即可开始使用。

```cpp
#include "dmdatetime.h"
#include <iostream>

int main() {
    // 1. 获取当前时间并格式化输出
    CDMDateTime now = CDMDateTime::Now();
    std::cout << "当前时间: " << now.ToString() << std::endl;
    std::cout << "中文格式: " << now.ToString(CDMDateTime::TO_STRING_STANDARD_CN) << std::endl;

    // 2. 创建一个特定日期
    CDMDateTime birthday(1999, 5, 20, 10, 30, 0);
    std::cout << "我的生日: " << birthday.ToString(CDMDateTime::TO_STRING_SHORT_DATE) << std::endl;

    // 3. 日期计算
    CDMDateTime future = now.AddDays(10).AddHours(5);
    std::cout << "10天5小时后: " << future.ToString() << std::endl;

    // 4. 计算时间差
    CDMTimeSpan age = now - birthday;
    std::cout << "我已经存在了 " << age.GetTotalDays() << " 天" << std::endl;

    // 5. 格式化为国际标准格式
    std::cout << "ISO 8601 格式: " << now.ToISOString() << std::endl;
    std::cout << "UTC 格式: " << now.ToUTCString() << std::endl;

    // 6. 使用便捷API获取下一个周五的上午9点
    CDMDateTime nextFriday = now.NextWeekdayAt(5, 9, 0, 0); // 星期日为0, 星期五为5
    std::cout << "下个周五上午9点: " << nextFriday.ToString() << std::endl;
    
    // 7. 获取月份的开始和结束
    CDMDateTime startOfMonth = now.GetStartOfMonth();
    CDMDateTime endOfMonth = now.GetEndOfMonth();
    std::cout << "本月第一天: " << startOfMonth.ToString() << std::endl;
    std::cout << "本月最后一天: " << endOfMonth.ToString() << std::endl;

    return 0;
}
```

## API 参考

### `CDMDateTime` 类

该类用于表示一个特定的时间点。

| 分类 | 函数原型 | 功能描述 |
| :--- | :--- | :--- |
| **构造与静态工厂** | `CDMDateTime()` | 默认构造函数，初始化为当前系统时间。 |
| | `CDMDateTime(year, month, day, hour, min, sec)` | 使用指定的年月日时分秒构造一个 `CDMDateTime` 对象。 |
| | `static CDMDateTime Now()` | 获取表示当前日期和时间的 `CDMDateTime` 对象。 |
| | `static CDMDateTime Today()` | 获取表示今天开始时间 (00:00:00) 的 `CDMDateTime` 对象。 |
| | `static CDMDateTime Parse(str, format)` | 从字符串按照指定格式解析日期时间。 |
| | `static CDMDateTime FromTimestamp(time_t)` | 从一个 `time_t` 类型的Unix时间戳创建一个 `CDMDateTime` 对象。 |
| | `static CDMDateTime MinValue()` | 获取此库支持的最小时间 (通常是 1970-01-01 00:00:00)。 |
| | `static CDMDateTime MaxValue()` | 获取此库支持的最大时间 (默认为 3000-01-01 00:00:00)。 |
| **设置值** | `SetDateTime(y, m, d, h, min, s)` | 设置对象的完整日期和时间。 |
| | `SetDate(y, m, d)` | 仅设置对象的日期部分，时间部分保持不变。 |
| | `SetTime(h, min, s)` | 仅设置对象的时间部分，日期部分保持不变。 |
| **获取时间分量** | `GetYear()`, `GetMonth()`, `GetDay()` | 获取年、月、日。 |
| | `GetHour()`, `GetMinute()`, `GetSecond()` | 获取时、分、秒。 |
| | `GetDayOfWeek()` | 获取星期几 (0=周日, 1=周一, ..., 6=周六)。 |
| | `GetDayOfYear()` | 获取这是一年中的第几天 (1-366)。 |
| | `GetTimestamp()` | 获取 `time_t` 格式的Unix时间戳（自Epoch以来的秒数）。 |
| **格式化** | `ToString(format)` | 将日期时间格式化为字符串。默认为 "YYYY-MM-DD HH:MM:SS"。 |
| | `ToUTCString()` | 将日期时间格式化为UTC时间的 ISO 8601 字符串 (以 'Z' 结尾)。 |
| | `ToISOString()` | 将日期时间格式化为带本地时区偏移的 ISO 8601 字符串。 |
| **算术运算** | `AddYears(n)`, `AddMonths(n)`, `AddDays(n)`... | 返回一个新的 `CDMDateTime` 对象，其值为当前对象增加指定的时间量。 |
| | `Subtract(other)` | 计算与另一个 `CDMDateTime` 对象的时间差，返回一个 `CDMTimeSpan` 对象。 |
| | `operator+(CDMTimeSpan)`, `operator-(CDMTimeSpan)` | 与 `CDMTimeSpan` 对象进行加减运算。 |
| **比较运算** | `operator<`, `operator>`, `operator<=`, `operator>=`, `==`, `!=` | 比较两个 `CDMDateTime` 对象的先后顺序。 |
| **边界获取** | `GetStartOfDay()`, `GetEndOfDay()` | 获取所在日期的开始时间 (00:00:00) 或结束时间 (23:59:59)。 |
| | `GetStartOfMonth()`, `GetEndOfMonth()` | 获取所在月份的开始或结束时间。 |
| | `GetStartOfYear()`, `GetEndOfYear()` | 获取所在年份的开始或结束时间。 |
| **属性判断** | `IsLeapYear()` | 判断当前对象的年份是否为闰年。 |
| | `IsWeekday()`, `IsWeekend()` | 判断当前对象是工作日还是周末。 |
| | `IsBetween(start, end, inclusive)` | 判断当前时间是否在指定的两个时间点之间。 |
| **便捷函数** | `TodayAt(h, m, s)` | 返回一个表示今天指定时间的 `CDMDateTime` 对象。 |
| | `TomorrowAt(h, m, s)` | 返回一个表示明天指定时间的 `CDMDateTime` 对象。 |
| | `YesterdayAt(h, m, s)` | 返回一个表示昨天指定时间的 `CDMDateTime` 对象。 |
| | `NextWeekdayAt(weekday, h, m, s)` | 获取下一个指定星期的具体时间。 |
| | `NextMonthOn(day, h, m, s)` | 获取下个月指定日期的具体时间。 |

### `CDMTimeSpan` 类

该类用于表示一个时间间隔或持续时间。

| 分类 | 函数原型 | 功能描述 |
| :--- | :--- | :--- |
| **构造** | `CDMTimeSpan(totalSeconds)` | 使用总秒数构造一个时间段对象。 |
| **获取时间分量** | `GetTotalDays()` | 获取此时间段表示的总天数 (向下取整)。 |
| | `GetTotalHours()` | 获取此时间段表示的总小时数 (向下取整)。 |
| | `GetTotalMinutes()` | 获取此时间段表示的总分钟数 (向下取整)。 |
| | `GetTotalSeconds()` | 获取此时间段表示的总秒数。 |
| **算术与比较** | `operator+`, `operator-`, `operator<`, `>`... | 对两个 `CDMTimeSpan` 对象进行加、减和大小比较。 |

## 许可证

本项目采用 [MIT License](https://opensource.org/licenses/MIT) 授权。详情请见文件头部的版权声明。