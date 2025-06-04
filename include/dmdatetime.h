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
#include <stdexcept>
#include <ctime> // Required for time_t, tm, mktime, localtime_r/s, gmtime_r/s, strftime, time
#include <cstdio>  // For snprintf
#include <cstring> // For C-style string operations (though not directly used extensively)
// Removed <chrono>, <iomanip> (unless needed for other parts, not for core logic here)
// <algorithm>, <vector> were included but not used directly by the classes.

class CDMDateTime;

class CDMTimeSpan {
private:
    time_t duration_seconds_;

public:
    explicit CDMTimeSpan(time_t totalSeconds = 0) : duration_seconds_(totalSeconds) {}

    long long GetTotalDays() const {
        return duration_seconds_ / (24LL * 60 * 60);
    }
    long long GetTotalHours() const {
        return duration_seconds_ / (60LL * 60);
    }
    long long GetTotalMinutes() const {
        return duration_seconds_ / 60LL;
    }
    time_t GetTotalSeconds() const {
        return duration_seconds_;
    }

    CDMTimeSpan operator+(const CDMTimeSpan& other) const {
        return CDMTimeSpan(duration_seconds_ + other.duration_seconds_);
    }
    CDMTimeSpan operator-(const CDMTimeSpan& other) const {
        return CDMTimeSpan(duration_seconds_ - other.duration_seconds_);
    }
    bool operator<(const CDMTimeSpan& other) const { return duration_seconds_ < other.duration_seconds_; }
    bool operator>(const CDMTimeSpan& other) const { return duration_seconds_ > other.duration_seconds_; }
    bool operator<=(const CDMTimeSpan& other) const { return duration_seconds_ <= other.duration_seconds_; }
    bool operator>=(const CDMTimeSpan& other) const { return duration_seconds_ >= other.duration_seconds_; }
    bool operator==(const CDMTimeSpan& other) const { return duration_seconds_ == other.duration_seconds_; }
    bool operator!=(const CDMTimeSpan& other) const { return duration_seconds_ != other.duration_seconds_; }
};

class CDMDateTime {
private:
    time_t time_t_value_;

    std::tm to_tm_local() const {
        std::tm local_tm{};
#ifdef _WIN32
        localtime_s(&local_tm, &time_t_value_);
#else
        localtime_r(&time_t_value_, &local_tm);
#endif
        return local_tm;
    }

    std::tm to_tm_utc() const {
        std::tm utc_tm{};
#ifdef _WIN32
        gmtime_s(&utc_tm, &time_t_value_);
#else
        gmtime_r(&time_t_value_, &utc_tm);
#endif
        return utc_tm;
    }

public:
    void SetDateTime(int year, int month, int day, int hour, int minute, int second) {
        std::tm t{};
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        t.tm_isdst = -1;

        time_t_value_ = std::mktime(&t);
        if (time_t_value_ == static_cast<time_t>(-1)) {
            throw std::runtime_error("Invalid date/time components that mktime could not normalize or represent.");
        }
    }

    void SetDate(int year, int month, int day) {
        std::tm t_current = to_tm_local();
        SetDateTime(year, month, day, t_current.tm_hour, t_current.tm_min, t_current.tm_sec);
    }

    void SetTime(int hour, int minute, int second) {
        std::tm t_current = to_tm_local();
        SetDateTime(t_current.tm_year + 1900, t_current.tm_mon + 1, t_current.tm_mday, hour, minute, second);
    }
private:
    explicit CDMDateTime(time_t t_val) : time_t_value_(t_val) {}

public:
    static CDMDateTime Now() {
        return CDMDateTime(std::time(nullptr));
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

        CDMDateTime resultDt; // Default constructor will init to now, but SetDateTime will override
        resultDt.SetDateTime(year, month, day, hour, minute, second);
        return resultDt;
    }

    static CDMDateTime FromTimestamp(time_t timestamp) {
        return CDMDateTime(timestamp);
    }
public:
    // These static const char* members require definition in a .cpp file.
    static const char* FORMAT_STANDARD;
    static const char* FORMAT_SHORT_DATE;
    static const char* FORMAT_STANDARD_CN;
    static const char* FORMAT_SHORT_DATE_CN;

    static const char* TO_STRING_STANDARD;
    static const char* TO_STRING_SHORT_DATE;
    static const char* TO_STRING_STANDARD_CN;
    static const char* TO_STRING_SHORT_DATE_CN;
    static const int DMDATETIME_YEAR_MAX = 3000;
    static const int DMDATETIME_YEAR_MIN = 1970;
    CDMDateTime() : time_t_value_(std::time(nullptr)) {}

    CDMDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
        SetDateTime(year, month, day, hour, minute, second);
    }

    std::string ToString(const std::string& format_string = TO_STRING_STANDARD) const {
        char buffer[128] = { 0 };
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
        return ToString(TO_STRING_STANDARD); // Original behavior, not strictly ISO 8601
    }

    std::string ToUTCString() const {
        char buffer[128] = { 0 };
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
    int GetDayOfWeek() const { return to_tm_local().tm_wday; } // 0=Sunday, 6=Saturday
    int GetDayOfYear() const { return to_tm_local().tm_yday + 1; } // tm_yday is 0-365

    CDMDateTime AddYears(int years) const {
        std::tm t = to_tm_local();
        t.tm_year += years;
        t.tm_isdst = -1;
        time_t tt = mktime(&t);
        if (tt == static_cast<time_t>(-1)) throw std::runtime_error("Resulting date out of range after AddYears");
        return CDMDateTime(tt);
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
        time_t tt = mktime(&t);
        if (tt == static_cast<time_t>(-1)) throw std::runtime_error("Resulting date out of range after AddMonths");
        return CDMDateTime(tt);
    }

    CDMDateTime AddDays(long long days) const {
        return CDMDateTime(time_t_value_ + days * 24LL * 60 * 60);
    }
    CDMDateTime AddHours(long long hours) const {
        return CDMDateTime(time_t_value_ + hours * 60LL * 60);
    }
    CDMDateTime AddMinutes(long long minutes) const {
        return CDMDateTime(time_t_value_ + minutes * 60LL);
    }
    CDMDateTime AddSeconds(long long seconds) const {
        return CDMDateTime(time_t_value_ + seconds);
    }

    CDMTimeSpan Subtract(const CDMDateTime& other) const {
        return CDMTimeSpan(time_t_value_ - other.time_t_value_);
    }

    CDMTimeSpan operator-(const CDMDateTime& other) const {
        return Subtract(other);
    }

    CDMDateTime operator+(const CDMTimeSpan& span) const {
        return CDMDateTime(time_t_value_ + span.GetTotalSeconds());
    }
    CDMDateTime operator-(const CDMTimeSpan& span) const {
        return CDMDateTime(time_t_value_ - span.GetTotalSeconds());
    }

    bool operator<(const CDMDateTime& other) const { return time_t_value_ < other.time_t_value_; }
    bool operator>(const CDMDateTime& other) const { return time_t_value_ > other.time_t_value_; }
    bool operator<=(const CDMDateTime& other) const { return time_t_value_ <= other.time_t_value_; }
    bool operator>=(const CDMDateTime& other) const { return time_t_value_ >= other.time_t_value_; }
    bool operator==(const CDMDateTime& other) const { return time_t_value_ == other.time_t_value_; }
    bool operator!=(const CDMDateTime& other) const { return time_t_value_ != other.time_t_value_; }

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
    CDMDateTime GetEndOfYear() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, 12, 31, 23, 59, 59);
    }

    bool IsLeapYear() const {
        int y = GetYear();
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
    bool IsWeekday() const {
        int dow = GetDayOfWeek();
        return dow >= 1 && dow <= 5; // Monday (1) to Friday (5)
    }
    bool IsWeekend() const {
        int dow = GetDayOfWeek();
        return dow == 0 || dow == 6; // Sunday (0) or Saturday (6)
    }
    time_t GetTimestamp() const {
        return time_t_value_;
    }

    static CDMDateTime Today() {
        return Now().GetStartOfDay();
    }

    static CDMDateTime MinValue() {
        return CDMDateTime(DMDATETIME_YEAR_MIN, 1, 1, 8, 0, 0); // Original value
    }

    static CDMDateTime MaxValue() {
        return CDMDateTime(DMDATETIME_YEAR_MAX, 1, 1, 8, 0, 0); // Original value
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

    CDMDateTime NextWeekdayAt(int target_weekday_tm_wday, int hour, int minute, int second) const {
        if (target_weekday_tm_wday < 0 || target_weekday_tm_wday > 6) {
            throw std::out_of_range("target_weekday_tm_wday must be between 0 (Sunday) and 6 (Saturday).");
        }
        int current_dow = GetDayOfWeek();
        int days_to_add = target_weekday_tm_wday - current_dow;
        if (days_to_add <= 0) {
            days_to_add += 7;
        }
        CDMDateTime target_day_date_part = AddDays(days_to_add);
        return CDMDateTime(target_day_date_part.GetYear(), target_day_date_part.GetMonth(), target_day_date_part.GetDay(), hour, minute, second);
    }

    CDMDateTime NextMonthOn(int day, int hour, int minute, int second) const {
        std::tm t_current = to_tm_local();
        int target_year = t_current.tm_year + 1900;
        int target_month_one_based = t_current.tm_mon + 1 + 1; // tm_mon is 0-11, +1 for current month 1-12, +1 for next month

        if (target_month_one_based > 12) { // e.g. current is Dec (11), mon+1 = 12. target_month = 12+1 = 13.
            target_year += (target_month_one_based - 1) / 12;
            target_month_one_based = (target_month_one_based - 1) % 12 + 1;
        }
        return CDMDateTime(target_year, target_month_one_based, day, hour, minute, second);
    }
    // ----- 新增接口结束 -----
};

// Definitions for static const char* members should be in a .cpp file:
const char* CDMDateTime::FORMAT_STANDARD = "%d-%d-%d %d:%d:%d";
const char* CDMDateTime::FORMAT_SHORT_DATE = "%d-%d-%d";
const char* CDMDateTime::FORMAT_STANDARD_CN = "%d年%d月%d日 %d时%d分%d秒";
const char* CDMDateTime::FORMAT_SHORT_DATE_CN = "%d年%d月%d日";
const char* CDMDateTime::TO_STRING_STANDARD = "%04d-%02d-%02d %02d:%02d:%02d";
const char* CDMDateTime::TO_STRING_SHORT_DATE = "%04d-%02d-%02d";
const char* CDMDateTime::TO_STRING_STANDARD_CN = "%04d年%02d月%02d日 %02d时%02d分%02d秒";
const char* CDMDateTime::TO_STRING_SHORT_DATE_CN = "%04d年%02d月%02d日";

#endif // __DMDATE_TIME_H__