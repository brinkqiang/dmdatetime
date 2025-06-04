# dmdatetime 🕒

A lightweight, cross-platform C++ datetime library for efficient date and time manipulation.

## Features ✨
- 🕰️ **Precision**: Supports milliseconds precision
- 🌐 **Timezone Handling**: UTC and local time conversions
- 📅 **Date Manipulation**: Add years, months, days, etc.
- ⏱️ **Time Span Calculations**: `CDMTimeSpan` for duration operations
- 📊 **Date Queries**: Day of week, day of year, leap year checks
- 📈 **Range Operations**: Check if datetime falls within range
- 🧪 **Boundary Values**: MinValue, MaxValue, Today helpers
- 🔄 **Parsing & Formatting**: Multiple standard formats support
- 🧩 **Cross-Platform**: Windows and Linux compatible

## Usage Examples 🚀

### Create DateTime
```cpp
CDMDateTime now = CDMDateTime::Now(); // Current time
CDMDateTime custom(2023, 12, 31, 23, 59, 59, 999); // Specific datetime
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
long long hours = duration.GetTotalHours();
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
| `FORMAT_STANDARD_MS` | "2023-12-31 23:59:59.999" |
| `FORMAT_SHORT_DATE` | "2023-12-31" |
| `FORMAT_STANDARD_CN` | "2023年12月31日 23时59分59秒" |
| `FORMAT_STANDARD_MS_CN` | "2023年12月31日 23时59分59秒.999" |

## License 📄
MIT License - See [LICENSE](LICENSE) for details.
