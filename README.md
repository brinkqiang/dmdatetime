# dmdatetime 🕒

A lightweight, cross-platform C++ datetime library for efficient date and time manipulation.

## Features ✨
- 📅 **Date/Time Manipulation**: Add years, months, days, etc. with new SetDate/SetTime methods
- ⏱️ **Time Span Calculations**: `CDMTimeSpan` for duration operations
- 📊 **Date Queries**: Day of week, day of year, leap year checks
- 📈 **Range Operations**: Check if datetime falls within range
- 🧪 **Boundary Values**: MinValue, MaxValue, Today helpers
- 🔄 **Parsing & Formatting**: Flexible formatting support
- 🧩 **Cross-Platform**: Windows and Linux compatible
- 🆕 **New Helpers**: TomorrowAt, YesterdayAt, TodayAt, NextWeekdayAt, NextMonthOn

## Usage Examples 🚀

### Create and Modify DateTime
```cpp
CDMDateTime now = CDMDateTime::Now(); // Current time
CDMDateTime custom(2023, 12, 31, 23, 59, 59); // Specific datetime

// Set date/time separately
custom.SetDate(2024, 1, 1); // Change to New Year
custom.SetTime(0, 0, 0);    // Midnight
```

### New Helper Methods
```cpp
// Get tomorrow at specific time
CDMDateTime meeting = now.TomorrowAt(14, 30, 0);

// Get next Monday at 9:00 AM
CDMDateTime nextMonday = now.NextWeekdayAt(1, 9, 0, 0);

// Get 15th of next month at noon
CDMDateTime nextMonth = now.NextMonthOn(15, 12, 0, 0);
```

### Formatting
```cpp
std::string iso = now.ToString(); // "2023-12-31 23:59:59"
std::string cnFormat = now.ToString(CDMDateTime::TO_STRING_STANDARD_CN);
```

### Date Manipulation
```cpp
CDMDateTime tomorrow = now.AddDays(1);
CDMDateTime nextMonth = now.AddMonths(1);
CDMDateTime startOfDay = now.GetStartOfDay();
```

### Time Span Calculations
```cpp
CDMTimeSpan duration = tomorrow.Subtract(now);
time_t hours = duration.GetTotalHours();
```

### Parsing
```cpp
CDMDateTime parsed = CDMDateTime::Parse("2023-12-31 23:59:59");
```

## Build Instructions 🔧
```bash
# Linux/macOS
./build.sh

# Windows
build.bat
```

## Format Specifiers 🧾
| Format | Description |
|--------|-------------|
| `FORMAT_STANDARD` | "2023-12-31 23:59:59" |
| `FORMAT_SHORT_DATE` | "2023-12-31" |
| `FORMAT_STANDARD_CN` | "2023年12月31日 23时59分59秒" |
| `TO_STRING_STANDARD` | "%04d-%02d-%02d %02d:%02d:%02d" |
| `TO_STRING_STANDARD_CN` | "%04d年%02d月%02d日 %02d时%02d分%02d秒" |

## License 📄
MIT License - See [LICENSE](LICENSE) for details.
