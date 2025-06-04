
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

#ifndef DMDATETIME_H_
#define DMDATETIME_H_

#include <string>
#include <chrono>
#include <iomanip> // For std::setw, std::setfill (still useful for some manual formatting)
// #include <sstream> // No longer used for ToString performance reasons
#include <stdexcept>
#include <ctime>
#include <algorithm> // For std::min, std::max
#include <vector>
#include <cstdio>    // For snprintf, sscanf
#include <cstring>   // For strlen, strcpy (if needed)
// #include <cassert> // Include if you plan to use assert() directly in your calling code

// Forward declaration
class CDMDateTime;

class CDMTimeSpan {
private:
    std::chrono::milliseconds duration_;

public:
    explicit CDMTimeSpan(std::chrono::milliseconds ms) : duration_(ms) {}
    CDMTimeSpan(long long totalMilliseconds = 0) : duration_(totalMilliseconds) {}

    long long GetTotalDays() const {
        return std::chrono::duration_cast<std::chrono::hours>(duration_).count() / 24;
    }
    long long GetTotalHours() const {
        return std::chrono::duration_cast<std::chrono::hours>(duration_).count();
    }
    long long GetTotalMinutes() const {
        return std::chrono::duration_cast<std::chrono::minutes>(duration_).count();
    }
    long long GetTotalSeconds() const {
        return std::chrono::duration_cast<std::chrono::seconds>(duration_).count();
    }
    long long GetTotalMilliseconds() const {
        return duration_.count();
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

    // Made public for Parse to use, but still primarily an internal helper
public: // Change to public if Parse needs to call it directly, or keep private and have Parse use public constructor
    void init_from_components(int year, int month, int day, int hour, int minute, int second, int millisecond) {
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
        time_point_ += std::chrono::milliseconds(std::max(0, std::min(millisecond, 999)));
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
    static const char* FORMAT_STANDARD_NO_MS;
    static const char* FORMAT_SHORT_DATE;
    static const char* FORMAT_LONG_DATE_CN;
    static const char* FORMAT_SHORT_TIME;
    static const char* FORMAT_LONG_TIME;
    static const char* FORMAT_ISO_UTC;
    static const char* FORMAT_FILENAME_SAFE; // yyyyMMdd_HHmmss


    CDMDateTime() : time_point_(std::chrono::system_clock::now()) {}

    static CDMDateTime Now() {
        return CDMDateTime(std::chrono::system_clock::now());
    }

    static CDMDateTime Parse(const std::string& dateTimeStr,const std::string& format_hint = "") {
        CDMDateTime resultDt;
        int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, millisecond = 0;
        int fields_scanned = 0;
        bool parsed_successfully = false;

        // Attempt parsing with various common formats using sscanf
        // Pattern 1: YYYY-MM-DD HH:MM:SS.ms
        fields_scanned = sscanf(dateTimeStr.c_str(), "%d-%d-%d %d:%d:%d.%d",
            &year, &month, &day, &hour, &minute, &second, &millisecond);
        if (fields_scanned >= 6 || fields_scanned == 3) { // Date and time mandatory, ms optional
            if (fields_scanned != 7) millisecond = 0;
            parsed_successfully = true;
        }

        // Pattern 2: YYYY-MM-DD HH:MM:SS (no ms)
        if (!parsed_successfully) {
            fields_scanned = sscanf(dateTimeStr.c_str(), "%d-%d-%d %d:%d:%d",
                &year, &month, &day, &hour, &minute, &second);
            if (fields_scanned == 6) {
                millisecond = 0;
                parsed_successfully = true;
            }
        }

        // Pattern 3: YYYY/MM/DD HH:MM:SS.ms
        if (!parsed_successfully) {
            fields_scanned = sscanf(dateTimeStr.c_str(), "%d/%d/%d %d:%d:%d.%d",
                &year, &month, &day, &hour, &minute, &second, &millisecond);
            if (fields_scanned >= 6) {
                if (fields_scanned == 6) millisecond = 0;
                parsed_successfully = true;
            }
        }

        // Pattern 4: YYYY/MM/DD HH:MM:SS (no ms)
        if (!parsed_successfully) {
            fields_scanned = sscanf(dateTimeStr.c_str(), "%d/%d/%d %d:%d:%d",
                &year, &month, &day, &hour, &minute, &second);
            if (fields_scanned == 6) {
                millisecond = 0;
                parsed_successfully = true;
            }
        }

        // Pattern 5: YYYYMMDD_HHMMSS (often used in filenames)
        if (!parsed_successfully && (format_hint == FORMAT_FILENAME_SAFE || format_hint == "yyyyMMdd_HHmmss")) {
            fields_scanned = sscanf(dateTimeStr.c_str(), "%4d%2d%2d_%2d%2d%2d",
                &year, &month, &day, &hour, &minute, &second);
            if (fields_scanned == 6) {
                millisecond = 0;
                parsed_successfully = true;
            }
        }


        if (parsed_successfully) {
            // Basic validation for component ranges (more thorough validation happens in mktime)
            if (month < 1 || month > 12 || day < 1 || day > 31 || // Simplistic day check
                hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
                second < 0 || second > 59 || // sscanf might read >59 for seconds like "60" which mktime handles
                millisecond < 0 || millisecond > 999) {
                // Optional: assert(false && "Parsed date/time components out of basic typical range");
                return resultDt;
            }
            try {
                // Use the public constructor which calls init_from_components
                resultDt = CDMDateTime(year, month, day, hour, minute, second, millisecond);

                return resultDt;
            }
            catch (const std::runtime_error&) {
                // mktime failed during construction
                // Optional: assert(false && "mktime failed for parsed components during CDMDateTime construction");
                return resultDt;
            }
        }

        // If format_hint was specific and we still failed, or all generic attempts failed.
        // Optional: assert(false && "DateTime Parse failed for input string");
        return resultDt;
    }


    CDMDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int millisecond = 0) {
        init_from_components(year, month, day, hour, minute, second, millisecond);
    }

    static CDMDateTime FromTimestamp(int64_t timestamp, bool isMilliseconds = false) {
        if (isMilliseconds) {
            return CDMDateTime(std::chrono::system_clock::time_point(std::chrono::milliseconds(timestamp)));
        }
        else {
            return CDMDateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
        }
    }

    std::string ToString(const std::string& formatStr = FORMAT_STANDARD_NO_MS) const {
        char buffer[128]; // Buffer for formatted string
        buffer[0] = '\0'; // Initialize to empty string
        std::tm t_local = to_tm_local();
        size_t len = 0;
        bool custom_handled = false;

        if (formatStr == FORMAT_STANDARD) {
            len = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &t_local);
            if (len > 0 && len < sizeof(buffer) - 4) { // Check space for ".fff"
                snprintf(buffer + len, sizeof(buffer) - len, ".%03d", GetMillisecond());
            }
            custom_handled = true;
        }
        else if (formatStr == FORMAT_ISO_UTC) {
            std::tm t_utc_val = to_tm_utc();
            len = strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &t_utc_val);
            if (len > 0 && len < sizeof(buffer) - 5) { // Check space for ".fffZ"
                snprintf(buffer + len, sizeof(buffer) - len, ".%03dZ", GetMillisecond());
            }
            custom_handled = true;
        }
        else if (formatStr == FORMAT_LONG_DATE_CN) {
            snprintf(buffer, sizeof(buffer), "%04d年%02d月%02d日",
                t_local.tm_year + 1900, t_local.tm_mon + 1, t_local.tm_mday);
            custom_handled = true;
        }
        else if (formatStr == FORMAT_FILENAME_SAFE || formatStr == "yyyyMMdd_HHmmss") {
            strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &t_local);
            custom_handled = true;
        }
        // Add more explicit format handling here if needed

        if (!custom_handled) { // Fallback for other strftime-compatible or semi-custom formats
            std::string tempFormatStr = formatStr;
            // Map user-friendly tokens to strftime specifiers
            replace_all(tempFormatStr, "yyyy", "%Y");
            replace_all(tempFormatStr, "yy", "%y");
            replace_all(tempFormatStr, "MM", "%m");
            replace_all(tempFormatStr, "dd", "%d");
            replace_all(tempFormatStr, "HH", "%H");
            replace_all(tempFormatStr, "hh", "%I");
            replace_all(tempFormatStr, "mm", "%M");
            replace_all(tempFormatStr, "ss", "%S");
            // 'fff' for milliseconds needs special handling after strftime

            bool has_fff = (tempFormatStr.find("fff") != std::string::npos);
            if (has_fff) {
                replace_all(tempFormatStr, ".fff", ""); // Remove .fff for strftime pass
                replace_all(tempFormatStr, "fff", "");  // Remove fff for strftime pass
            }

            len = strftime(buffer, sizeof(buffer), tempFormatStr.c_str(), &t_local);

            if (len > 0 && has_fff) {
                if (formatStr.find(".fff") != std::string::npos) { // Original format had ".fff"
                    if (len < sizeof(buffer) - 4) {
                        snprintf(buffer + len, sizeof(buffer) - len, ".%03d", GetMillisecond());
                    }
                }
                else if (formatStr.find("fff") != std::string::npos) { // Original format had "fff"
                    if (len < sizeof(buffer) - 3) {
                        snprintf(buffer + len, sizeof(buffer) - len, "%03d", GetMillisecond());
                    }
                }
            }
        }

        if (strlen(buffer) == 0 && sizeof(buffer) > 0) { // strftime/snprintf failed or wrote nothing
            // Consider returning a specific error indicator or an empty string safely
        }
        return std::string(buffer);
    }


    std::string ToISOString() const {
        return ToString(FORMAT_ISO_UTC);
    }

    std::string ToUTCString() const { // Default "yyyy-MM-dd HH:mm:ss.fff UTC"
        char buffer[128];
        std::tm t_utc_val = to_tm_utc();
        size_t len = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &t_utc_val);
        if (len > 0 && len < sizeof(buffer) - 8) { // Space for ".fff UTC"
            snprintf(buffer + len, sizeof(buffer) - len, ".%03d UTC", GetMillisecond());
        }
        else {
            buffer[0] = '\0'; // Safety for failure
        }
        return std::string(buffer);
    }

    int GetYear() const { return to_tm_local().tm_year + 1900; }
    int GetMonth() const { return to_tm_local().tm_mon + 1; }
    int GetDay() const { return to_tm_local().tm_mday; }
    int GetHour() const { return to_tm_local().tm_hour; }
    int GetMinute() const { return to_tm_local().tm_min; }
    int GetSecond() const { return to_tm_local().tm_sec; }
    int GetMillisecond() const {
        auto since_epoch = time_point_.time_since_epoch();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch - sec);
        return static_cast<int>(ms.count());
    }
    int GetDayOfWeek() const { return to_tm_local().tm_wday; }
    int GetDayOfYear() const { return to_tm_local().tm_yday + 1; }

    CDMDateTime AddYears(int years) const {
        std::tm t = to_tm_local();
        t.tm_year += years;
        t.tm_isdst = -1;
        std::time_t tt = mktime(&t);
        if (tt == -1) throw std::runtime_error("Resulting date out of range after AddYears");
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt) + std::chrono::milliseconds(GetMillisecond()));
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
        return CDMDateTime(std::chrono::system_clock::from_time_t(tt) + std::chrono::milliseconds(GetMillisecond()));
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
    CDMDateTime AddMilliseconds(int ms) const {
        return CDMDateTime(time_point_ + std::chrono::milliseconds(ms));
    }

    CDMTimeSpan Subtract(const CDMDateTime& other) const {
        return CDMTimeSpan(std::chrono::duration_cast<std::chrono::milliseconds>(time_point_ - other.time_point_));
    }

    bool operator<(const CDMDateTime& other) const { return time_point_ < other.time_point_; }
    bool operator>(const CDMDateTime& other) const { return time_point_ > other.time_point_; }
    bool operator<=(const CDMDateTime& other) const { return time_point_ <= other.time_point_; }
    bool operator>=(const CDMDateTime& other) const { return time_point_ >= other.time_point_; }
    bool operator==(const CDMDateTime& other) const { return time_point_ == other.time_point_; }
    bool operator!=(const CDMDateTime& other) const { return time_point_ != other.time_point_; }

    CDMDateTime GetStartOfDay() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 0, 0, 0, 0);
    }
    CDMDateTime GetEndOfDay() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 23, 59, 59, 999);
    }
    CDMDateTime GetStartOfMonth() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, t.tm_mon + 1, 1, 0, 0, 0, 0);
    }
    CDMDateTime GetEndOfMonth() const {
        std::tm t = to_tm_local();
        int year = t.tm_year + 1900;
        int month = t.tm_mon + 1;
        if (month == 12) {
            return CDMDateTime(year, 12, 31, 23, 59, 59, 999);
        }
        else {
            CDMDateTime startOfNextMonth(year, month + 1, 1, 0, 0, 0, 0);
            return startOfNextMonth.AddMilliseconds(-1);
        }
    }
    CDMDateTime GetStartOfYear() const {
        std::tm t = to_tm_local();
        return CDMDateTime(t.tm_year + 1900, 1, 1, 0, 0, 0, 0);
    }

    CDMDateTime ToUTC() const { return *this; }
    CDMDateTime ToLocal() const { return *this; }
    CDMDateTime ToTimeZone(int /* utc_offset_hours */) const { return *this; }

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
    int64_t GetTimestamp() const {
        return std::chrono::duration_cast<std::chrono::seconds>(time_point_.time_since_epoch()).count();
    }
    int64_t GetTimestampMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count();
    }

    static CDMDateTime Today() {
        return Now().GetStartOfDay();
    }
    static CDMDateTime MinValue() {
        return CDMDateTime(1970, 1, 1, 8, 0, 0, 0);
    }
    static CDMDateTime MaxValue() {
        if (sizeof(std::time_t) > 4) {
            return CDMDateTime(3000, 12, 31, 23, 59, 59, 999);
        }
        else {
            return CDMDateTime(2038, 1, 19, 3, 14, 7, 0); // Near 32-bit time_t limit
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

const char* CDMDateTime::FORMAT_STANDARD = "yyyy-MM-dd HH:mm:ss.fff";
const char* CDMDateTime::FORMAT_STANDARD_NO_MS = "%Y-%m-%d %H:%M:%S";
const char* CDMDateTime::FORMAT_SHORT_DATE = "%Y-%m-%d";
const char* CDMDateTime::FORMAT_LONG_DATE_CN = "yyyy年MM月dd日";
const char* CDMDateTime::FORMAT_SHORT_TIME = "%H:%M";
const char* CDMDateTime::FORMAT_LONG_TIME = "%H:%M:%S";
const char* CDMDateTime::FORMAT_ISO_UTC = "yyyy-MM-ddTHH:mm:ss.fffZ";
const char* CDMDateTime::FORMAT_FILENAME_SAFE = "yyyyMMdd_HHmmss";

#endif // DMDATETIME_H_