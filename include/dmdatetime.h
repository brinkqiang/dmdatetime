
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
    void init_from_components(int year, int month, int day, int hour, int minute, int second) {
        std::tm t{};
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        t.tm_isdst = -1;

        std::time_t tt = mktime(&t);
        if (tt == (std::time_t)(-1)) {
            throw std::runtime_error("Invalid date/time components that mktime could not normalize or represent.");
        }
        time_point_ = std::chrono::system_clock::from_time_t(tt);
    }
private:

    explicit CDMDateTime(std::chrono::system_clock::time_point tp) : time_point_(tp) {}

    static void replace_all(std::string& str, const std::string& from, const std::string& to) {
        if (from.empty()) return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
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
        // Ensure time_point_ is at second precision if default constructed
        time_point_ = std::chrono::time_point_cast<std::chrono::seconds>(time_point_);
    }

    static CDMDateTime Now() {
        auto now_tp = std::chrono::system_clock::now();
        // Cast to seconds precision
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
        resultDt.init_from_components(year, month, day, hour, minute, second);
        return resultDt;
    }

    CDMDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
        init_from_components(year, month, day, hour, minute, second);
    }

    static CDMDateTime FromTimestamp(time_t timestamp) {
        return CDMDateTime(std::chrono::system_clock::from_time_t(timestamp));
    }

    std::string ToString(const std::string& format_string = "%04d-%02d-%02d %02d:%02d:%02d") const {
        char buffer[128];
        std::tm t_local = to_tm_local();

        // IMPORTANT: The 'format_string' argument is now expected to be a snprintf-compatible format string.
        // The original default "%Y-%m-%d %H:%M:%S" is a strftime format and will NOT work as expected
        // with snprintf directly. snprintf would try to print 'Y', '-', 'm', etc., literally,
        // unless the format string is changed by the caller to something like:
        // "%04d-%02d-%02d %02d:%02d:%02d"
        //
        // The following arguments are always passed to snprintf in this order:
        // 1. Year (e.g., 2024)
        // 2. Month (1-12)
        // 3. Day (1-31)
        // 4. Hour (0-23)
        // 5. Minute (0-59)
        // 6. Second (0-59)
        // The provided format_string must correctly reference these arguments using snprintf specifiers (e.g., %d, %02d).

        snprintf(buffer, sizeof(buffer), format_string.c_str(),
            t_local.tm_year + 1900, // 1. Year
            t_local.tm_mon + 1,     // 2. Month
            t_local.tm_mday,        // 3. Day
            t_local.tm_hour,        // 4. Hour
            t_local.tm_min,         // 5. Minute
            t_local.tm_sec          // 6. Second
        );
        return std::string(buffer);
    }

    std::string ToISOString() const {
        return ToString(); // Default format is YYYY-MM-DD HH:MM:SS
    }

    std::string ToUTCString() const { // Default "YYYY-MM-DD HH:MM:SS UTC"
        char buffer[128];
        std::tm t_utc_val = to_tm_utc();
        size_t len = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S UTC", &t_utc_val);
        if (len == 0) { // Safety for strftime failure
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
    int GetDayOfWeek() const { return to_tm_local().tm_wday; } // Sunday is 0, Saturday is 6
    int GetDayOfYear() const { return to_tm_local().tm_yday + 1; } // tm_yday is 0-365

    CDMDateTime AddYears(int years) const {
        std::tm t = to_tm_local();
        t.tm_year += years;
        t.tm_isdst = -1; // Let mktime determine DST
        std::time_t tt = mktime(&t);
        if (tt == -1) throw std::runtime_error("Resulting date out of range after AddYears");
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt));
    }

    CDMDateTime AddMonths(int months) const {
        std::tm t = to_tm_local();
        int new_month_tm = t.tm_mon + months;
        t.tm_year += new_month_tm / 12; // Add years from month overflow
        t.tm_mon = new_month_tm % 12;   // Normalize month
        if (t.tm_mon < 0) {             // Handle negative months
            t.tm_mon += 12;
            t.tm_year--;
        }
        // mktime will also handle day adjustment if the new month has fewer days (e.g., Jan 31 + 1 month -> Feb 28/29)
        t.tm_isdst = -1;
        std::time_t tt = mktime(&t);
        if (tt == -1) throw std::runtime_error("Resulting date out of range after AddMonths");
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt));
    }

    CDMDateTime AddDays(int days) const {
        // std::chrono::hours can be constructed from a long long.
        // 24LL is used to ensure the multiplication is done using long long.
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
        // duration_cast to seconds, then construct CDMTimeSpan which expects seconds
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
        int month = t.tm_mon + 1; // tm_mon is 0-11, so month is 1-12
        if (month == 12) {
            return CDMDateTime(year, 12, 31, 23, 59, 59);
        }
        else {
            // Create the first day of the next month, then subtract one second.
            CDMDateTime startOfNextMonth(year, month + 1, 1, 0, 0, 0);
            return startOfNextMonth.AddSeconds(-1);
        }
    }
    CDMDateTime GetStartOfYear() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, 1, 1, 0, 0, 0);
    }

    // These methods are now essentially no-ops as time_point_ is assumed to be UTC
    // based on std::chrono::system_clock and lack of explicit time zone handling.
    // If true time zone conversion were needed, these would be more complex.
    CDMDateTime ToUTC() const { return *this; }
    CDMDateTime ToLocal() const { return *this; }
    CDMDateTime ToTimeZone(int /* utc_offset_hours */) const { return *this; } // Placeholder

    bool IsLeapYear() const {
        int y = GetYear();
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
    bool IsWeekday() const {
        int dow = GetDayOfWeek(); // Sunday (0) to Saturday (6)
        return dow >= 1 && dow <= 5; // Monday to Friday
    }
    bool IsWeekend() const {
        int dow = GetDayOfWeek();
        return dow == 0 || dow == 6; // Sunday or Saturday
    }
    time_t GetTimestamp() const { // Returns seconds since epoch (UTC)
        return std::chrono::system_clock::to_time_t(time_point_);
    }

    static CDMDateTime Today() {
        return Now().GetStartOfDay();
    }

    // MinValue for time_t often corresponds to the start of the Unix epoch.
    // For system_clock, it can represent times before the epoch if the clock supports it.
    // For simplicity and broad compatibility, we'll stick to epoch start.
    static CDMDateTime MinValue() {
        // Corresponds to 1970-01-01 00:00:00 UTC if time_t is Unix time
        return CDMDateTime(1970, 1, 1, 8, 0, 0);
    }

    // MaxValue depends on whether time_t is 32-bit or 64-bit.
    static CDMDateTime MaxValue() {
        if (sizeof(std::time_t) > 4) { // Likely 64-bit time_t
            // A common practical limit far in the future for 64-bit systems
            // (e.g. year 3000, well beyond typical 64-bit time_t limits like year 292 billion)
            return CDMDateTime(3000, 12, 31, 23, 59, 59);
        }
        else { // Likely 32-bit time_t (Y2038 problem)
            // Max value for 32-bit signed time_t is 2038-01-19 03:14:07 UTC
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
};

// Definitions for static const char* members
const char* CDMDateTime::FORMAT_STANDARD = "%d-%d-%d %d:%d:%d";
const char* CDMDateTime::FORMAT_SHORT_DATE = "%d-%d-%d";
const char* CDMDateTime::FORMAT_STANDARD_CN = "%d年%d月%d日 %d时%d分%d秒";
const char* CDMDateTime::FORMAT_SHORT_DATE_CN = "%d年%d月%d日";

const char* CDMDateTime::TO_STRING_STANDARD = "%04d-%02d-%02d %02d:%02d:%02d";
const char* CDMDateTime::TO_STRING_SHORT_DATE = "%04d-%02d-%02d";
const char* CDMDateTime::TO_STRING_STANDARD_CN = "%04d年%02d月%02d日 %02d时%02d分%02d秒";
const char* CDMDateTime::TO_STRING_SHORT_DATE_CN = "%04d年%02d月%02d日";

#endif // __DMDATE_TIME_H__
