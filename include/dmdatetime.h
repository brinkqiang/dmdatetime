// Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __DMDATE_TIME_H__
#define __DMDATE_TIME_H__

#include <string>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <ctime> // Required for time_t and tm functions
#include <algorithm>
#include <vector>
#include <cstdio>
#include <cstring>

class CDMDateTime;

class CDMTimeSpan {
private:
    std::chrono::seconds duration_;

public:
    explicit CDMTimeSpan(std::chrono::seconds s) : duration_(s) {}
    CDMTimeSpan(time_t totalSeconds = 0) : duration_(totalSeconds) {}

    long long GetTotalDays() const {
        return std::chrono::duration_cast<std::chrono::hours>(duration_).count() / 24;
    }
    long long GetTotalHours() const {
        return std::chrono::duration_cast<std::chrono::hours>(duration_).count();
    }
    long long GetTotalMinutes() const {
        return std::chrono::duration_cast<std::chrono::minutes>(duration_).count();
    }
    time_t GetTotalSeconds() const {
        return static_cast<time_t>(duration_.count());
    }

    CDMTimeSpan operator+(const CDMTimeSpan& other) const {
        return CDMTimeSpan(duration_ + other.duration_);
    }
    CDMTimeSpan operator-(const CDMTimeSpan& other) const {
        return CDMTimeSpan(duration_ - other.duration_);
    }
    bool operator<(const CDMTimeSpan& other) const { return duration_ < other.duration_; }
    bool operator>(const CDMTimeSpan& other) const { return duration_ > other.duration_; }
    bool operator<=(const CDMTimeSpan& other) const { return duration_ <= other.duration_; }
    bool operator>=(const CDMTimeSpan& other) const { return duration_ >= other.duration_; }
    bool operator==(const CDMTimeSpan& other) const { return duration_ == other.duration_; }
    bool operator!=(const CDMTimeSpan& other) const { return duration_ != other.duration_; }
};

class CDMDateTime {
private:
    std::chrono::system_clock::time_point time_point_;

    std::tm to_tm_local() const {
        std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
        std::tm local_tm{};
#ifdef _WIN32
        localtime_s(&local_tm, &tt);
#else
        localtime_r(&tt, &local_tm);
#endif
        return local_tm;
    }

    std::tm to_tm_utc() const {
        std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
        std::tm utc_tm{};
#ifdef _WIN32
        gmtime_s(&utc_tm, &tt);
#else
        gmtime_r(&tt, &utc_tm);
#endif
        return utc_tm;
    }

public:
    // 原 init_from_components 函数，建议更名为 SetDateTime
    void SetDateTime(int year, int month, int day, int hour, int minute, int second) {
        std::tm t{};
        t.tm_year = year - 1900;    // tm_year 是自1900年起的年数
        t.tm_mon = month - 1;       // tm_mon 是从0开始的月份 (0=一月, 11=十二月)
        t.tm_mday = day;            // tm_mday 是一月中的日期 (1-31)
        t.tm_hour = hour;           // tm_hour 是一天中的小时 (0-23)
        t.tm_min = minute;          // tm_min 是一小时中的分钟 (0-59)
        t.tm_sec = second;          // tm_sec 是一分钟中的秒数 (0-59)
        t.tm_isdst = -1;            // 夏令时信息，-1表示让系统自动判断

        std::time_t tt = std::mktime(&t); // 将 std::tm 结构转换为 time_t 类型
        if (tt == (std::time_t)(-1)) {
            // 如果 mktime 无法规范化或表示给定的日期/时间组件，则抛出异常
            throw std::runtime_error("Invalid date/time components that mktime could not normalize or represent.");
        }
        // time_point_ (类的成员变量) 被更新为新的时间点
        time_point_ = std::chrono::system_clock::from_time_t(tt);
    }

    // SetDate 函数应为 CDMDateTime 类的成员函数
    void SetDate(int year, int month, int day) {
        // 首先获取当前时间的小时、分钟和秒
        std::tm t_current = to_tm_local(); // to_tm_local() 是一个将 time_point_ 转换为本地 std::tm 结构的成员函数

        // 使用新的日期和当前的时间调用 SetDateTime
        SetDateTime(year, month, day, t_current.tm_hour, t_current.tm_min, t_current.tm_sec);
    }

    // SetTime 函数应为 CDMDateTime 类的成员函数
    void SetTime(int hour, int minute, int second) {
        // 首先获取当前时间的年、月、日
        std::tm t_current = to_tm_local(); // to_tm_local() 是一个将 time_point_ 转换为本地 std::tm 结构的成员函数

        // 使用新的时间和当前的日期调用 SetDateTime
        // 注意：t_current.tm_year 是自1900年起的年数, t_current.tm_mon 是0-11的月份
        SetDateTime(t_current.tm_year + 1900, t_current.tm_mon + 1, t_current.tm_mday, hour, minute, second);
    }
private:

    explicit CDMDateTime(std::chrono::system_clock::time_point tp) : time_point_(tp) {}

public:
    // static

    static CDMDateTime Now() {
        auto now_tp = std::chrono::system_clock::now();
        return CDMDateTime(std::chrono::time_point_cast<std::chrono::seconds>(now_tp));
    }

    static CDMDateTime Parse(const std::string& dateTimeStr, const std::string& sscanf_format = FORMAT_STANDARD) {
        int year = 0, month = 0, day = 0;
        int hour = 0, minute = 0, second = 0;

        int fields_scanned = sscanf(dateTimeStr.c_str(), sscanf_format.c_str(),
            &year, &month, &day,
            &hour, &minute, &second);

        if (fields_scanned == EOF || fields_scanned < 3) {
            char error_buf[256];
            snprintf(error_buf, sizeof(error_buf),
                "Failed to parse basic date components (Y,M,D) with sscanf. Format: '%s', Input: '%s'. Fields scanned: %d",
                sscanf_format.c_str(), dateTimeStr.c_str(), fields_scanned);
            throw std::runtime_error(error_buf);
        }

        if (day == 0 && fields_scanned >= 3) day = 1;

        CDMDateTime resultDt;
        resultDt.SetDateTime(year, month, day, hour, minute, second);
        return resultDt;
    }

    static CDMDateTime FromTimestamp(time_t timestamp) {
        return CDMDateTime(std::chrono::system_clock::from_time_t(timestamp));
    }
public:
    static const char* FORMAT_STANDARD;
    static const char* FORMAT_SHORT_DATE;
    static const char* FORMAT_STANDARD_CN;
    static const char* FORMAT_SHORT_DATE_CN;

    static const char* TO_STRING_STANDARD;
    static const char* TO_STRING_SHORT_DATE;
    static const char* TO_STRING_STANDARD_CN;
    static const char* TO_STRING_SHORT_DATE_CN;

    CDMDateTime() : time_point_(std::chrono::system_clock::now()) {
        time_point_ = std::chrono::time_point_cast<std::chrono::seconds>(time_point_);
    }


    CDMDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
        SetDateTime(year, month, day, hour, minute, second);
    }


    std::string ToString(const std::string& format_string = "%04d-%02d-%02d %02d:%02d:%02d") const {
        char buffer[128] = {0};
        std::tm t_local = to_tm_local();
        snprintf(buffer, sizeof(buffer), format_string.c_str(),
            t_local.tm_year + 1900,
            t_local.tm_mon + 1,
            t_local.tm_mday,
            t_local.tm_hour,
            t_local.tm_min,
            t_local.tm_sec
        );
        return std::string(buffer);
    }

    std::string ToISOString() const {
        return ToString();
    }

    std::string ToUTCString() const {
        char buffer[128];
        std::tm t_utc_val = to_tm_utc();
        size_t len = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", &t_utc_val);
        if (len == 0) {
            buffer[0] = '\0';
        }
        return std::string(buffer);
    }

    int GetYear() const { return to_tm_local().tm_year + 1900; }
    int GetMonth() const { return to_tm_local().tm_mon + 1; }
    int GetDay() const { return to_tm_local().tm_mday; }
    int GetHour() const { return to_tm_local().tm_hour; }
    int GetMinute() const { return to_tm_local().tm_min; }
    int GetSecond() const { return to_tm_local().tm_sec; }
    int GetDayOfWeek() const { return to_tm_local().tm_wday; }
    int GetDayOfYear() const { return to_tm_local().tm_yday + 1; }

    CDMDateTime AddYears(int years) const {
        std::tm t = to_tm_local();
        t.tm_year += years;
        t.tm_isdst = -1;
        std::time_t tt = mktime(&t);
        if (tt == -1) throw std::runtime_error("Resulting date out of range after AddYears");
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt));
    }

    CDMDateTime AddMonths(int months) const {
        std::tm t = to_tm_local();
        int new_month_tm = t.tm_mon + months;
        t.tm_year += new_month_tm / 12;
        t.tm_mon = new_month_tm % 12;
        if (t.tm_mon < 0) {
            t.tm_mon += 12;
            t.tm_year--;
        }
        t.tm_isdst = -1;
        std::time_t tt = mktime(&t);
        if (tt == -1) throw std::runtime_error("Resulting date out of range after AddMonths");
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt));
    }

    CDMDateTime AddDays(int days) const {
        return CDMDateTime(time_point_ + std::chrono::hours(days * 24LL));
    }
    CDMDateTime AddHours(int hours) const {
        return CDMDateTime(time_point_ + std::chrono::hours(hours));
    }
    CDMDateTime AddMinutes(int minutes) const {
        return CDMDateTime(time_point_ + std::chrono::minutes(minutes));
    }
    CDMDateTime AddSeconds(int seconds) const {
        return CDMDateTime(time_point_ + std::chrono::seconds(seconds));
    }

    CDMTimeSpan Subtract(const CDMDateTime& other) const {
        auto diff_seconds = std::chrono::duration_cast<std::chrono::seconds>(time_point_ - other.time_point_);
        return CDMTimeSpan(diff_seconds);
    }

    bool operator<(const CDMDateTime& other) const { return time_point_ < other.time_point_; }
    bool operator>(const CDMDateTime& other) const { return time_point_ > other.time_point_; }
    bool operator<=(const CDMDateTime& other) const { return time_point_ <= other.time_point_; }
    bool operator>=(const CDMDateTime& other) const { return time_point_ >= other.time_point_; }
    bool operator==(const CDMDateTime& other) const { return time_point_ == other.time_point_; }
    bool operator!=(const CDMDateTime& other) const { return time_point_ != other.time_point_; }


    CDMDateTime GetStartOfDay() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 0, 0, 0);
    }
    CDMDateTime GetEndOfDay() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 23, 59, 59);
    }
    CDMDateTime GetStartOfMonth() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, 1, 0, 0, 0);
    }
    CDMDateTime GetEndOfMonth() const {
        std::tm t = to_tm_local();
        int year = t.tm_year + 1900;
        int month = t.tm_mon + 1;
        if (month == 12) {
            return CDMDateTime(year, 12, 31, 23, 59, 59);
        }
        else {
            CDMDateTime startOfNextMonth(year, month + 1, 1, 0, 0, 0);
            return startOfNextMonth.AddSeconds(-1);
        }
    }
    CDMDateTime GetStartOfYear() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, 1, 1, 0, 0, 0);
    }

    bool IsLeapYear() const {
        int y = GetYear();
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
    bool IsWeekday() const {
        int dow = GetDayOfWeek();
        return dow >= 1 && dow <= 5;
    }
    bool IsWeekend() const {
        int dow = GetDayOfWeek();
        return dow == 0 || dow == 6;
    }
    time_t GetTimestamp() const {
        return std::chrono::system_clock::to_time_t(time_point_);
    }

    static CDMDateTime Today() {
        return Now().GetStartOfDay();
    }

    static CDMDateTime MinValue() {
        return CDMDateTime(1970, 1, 1, 8, 0, 0);
    }

    static CDMDateTime MaxValue() {
        if (sizeof(std::time_t) > 4) {
            return CDMDateTime(3000, 12, 1, 23, 59, 59);
        }
        else {
            return CDMDateTime(2038, 1, 19, 3, 14, 7);
        }
    }

    bool IsBetween(const CDMDateTime& start, const CDMDateTime& end, bool inclusiveStart = true, bool inclusiveEnd = true) const {
        bool checkLower = inclusiveStart ? (*this >= start) : (*this > start);
        if (!checkLower) return false;
        bool checkUpper = inclusiveEnd ? (*this <= end) : (*this < end);
        return checkUpper;
    }

    std::string ToLunarString() const {
        return "农历支持未实现 (Lunar support not implemented)";
    }

    // ----- 新增接口 -----
    CDMDateTime TomorrowAt(int hour, int minute, int second) const {
        CDMDateTime tomorrow_date_part = AddDays(1);
        return CDMDateTime(tomorrow_date_part.GetYear(), tomorrow_date_part.GetMonth(), tomorrow_date_part.GetDay(), hour, minute, second);
    }

    CDMDateTime YesterdayAt(int hour, int minute, int second) const {
        CDMDateTime yesterday_date_part = AddDays(-1);
        return CDMDateTime(yesterday_date_part.GetYear(), yesterday_date_part.GetMonth(), yesterday_date_part.GetDay(), hour, minute, second);
    }

    CDMDateTime TodayAt(int hour, int minute, int second) const {
        return CDMDateTime(GetYear(), GetMonth(), GetDay(), hour, minute, second);
    }

    CDMDateTime NextWeekdayAt(int target_weekday, int hour, int minute, int second) const {
        if (target_weekday < 0 || target_weekday > 6) {
            throw std::out_of_range("target_weekday must be between 0 (Sunday) and 6 (Saturday).");
        }
        int current_dow = GetDayOfWeek();
        int days_to_add = target_weekday - current_dow;
        if (days_to_add <= 0) {
            days_to_add += 7;
        }
        CDMDateTime target_day_date_part = AddDays(days_to_add);
        return CDMDateTime(target_day_date_part.GetYear(), target_day_date_part.GetMonth(), target_day_date_part.GetDay(), hour, minute, second);
    }

    CDMDateTime NextMonthOn(int day, int hour, int minute, int second) const {
        std::tm t_current = to_tm_local();

        int next_m_year = t_current.tm_year + 1900;
        int next_m_mon_idx = t_current.tm_mon + 1;

        if (next_m_mon_idx >= 12) {
            next_m_year += next_m_mon_idx / 12;
            next_m_mon_idx %= 12;
        }

        return CDMDateTime(next_m_year, next_m_mon_idx + 1, day, hour, minute, second);
    }
    // ----- 新增接口结束 -----
};

const char* CDMDateTime::FORMAT_STANDARD = "%d-%d-%d %d:%d:%d";
const char* CDMDateTime::FORMAT_SHORT_DATE = "%d-%d-%d";
const char* CDMDateTime::FORMAT_STANDARD_CN = "%d年%d月%d日 %d时%d分%d秒";
const char* CDMDateTime::FORMAT_SHORT_DATE_CN = "%d年%d月%d日";

const char* CDMDateTime::TO_STRING_STANDARD = "%04d-%02d-%02d %02d:%02d:%02d";
const char* CDMDateTime::TO_STRING_SHORT_DATE = "%04d-%02d-%02d";
const char* CDMDateTime::TO_STRING_STANDARD_CN = "%04d年%02d月%02d日 %02d时%02d分%02d秒";
const char* CDMDateTime::TO_STRING_SHORT_DATE_CN = "%04d年%02d月%02d日";

#endif // __DMDATE_TIME_H__