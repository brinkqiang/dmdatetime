
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
#include "libdmdatetime_impl.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

CDMDateTimeImpl::CDMDateTimeImpl() 
    : m_timezoneOffset(0), m_isValid(false) {
    SetToNow();
}

void CDMDateTimeImpl::Release(void) {
    delete this;
}

std::tm CDMDateTimeImpl::GetLocalTm() const {
    auto time_t_val = std::chrono::system_clock::to_time_t(m_timePoint);
    std::tm tm_val;
#ifdef _WIN32
    localtime_s(&tm_val, &time_t_val);
#else
    localtime_r(&time_t_val, &tm_val);
#endif
    return tm_val;
}

void CDMDateTimeImpl::SetFromTm(const std::tm& tm) {
    auto time_t_val = std::mktime(const_cast<std::tm*>(&tm));
    if (time_t_val != -1) {
        m_timePoint = std::chrono::system_clock::from_time_t(time_t_val);
        m_isValid = true;
    } else {
        m_isValid = false;
    }
}

bool CDMDateTimeImpl::IsValidDate(int year, int month, int day) const {
    if (year < 1900 || year > 9999) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1) return false;
    
    int daysInMonth = GetDaysInMonth(year, month);
    return day <= daysInMonth;
}

bool CDMDateTimeImpl::IsValidTime(int hour, int minute, int second) const {
    return hour >= 0 && hour < 24 && 
           minute >= 0 && minute < 60 && 
           second >= 0 && second < 60;
}

void CDMDateTimeImpl::SetDateTime(int year, int month, int day, int hour, int minute, int second) {
    if (!IsValidDate(year, month, day) || !IsValidTime(hour, minute, second)) {
        m_isValid = false;
        return;
    }
    
    std::tm tm_val = {};
    tm_val.tm_year = year - 1900;
    tm_val.tm_mon = month - 1;
    tm_val.tm_mday = day;
    tm_val.tm_hour = hour;
    tm_val.tm_min = minute;
    tm_val.tm_sec = second;
    tm_val.tm_isdst = -1;
    
    SetFromTm(tm_val);
}

void CDMDateTimeImpl::SetFromTimestamp(time_t timestamp) {
    m_timePoint = std::chrono::system_clock::from_time_t(timestamp);
    m_isValid = true;
}

void CDMDateTimeImpl::SetFromString(const std::string& dateTimeStr, DMDateTimeFormat format) {
    m_isValid = ParseFromString(dateTimeStr, format);
}

void CDMDateTimeImpl::SetToNow() {
    m_timePoint = std::chrono::system_clock::now();
    m_isValid = true;
}

bool CDMDateTimeImpl::ParseFromString(const std::string& str, DMDateTimeFormat format) {
    std::tm tm_val = {};
    
    switch (format) {
        case DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS: {
            // 2024-01-01 12:30:45
            if (sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", 
                      &tm_val.tm_year, &tm_val.tm_mon, &tm_val.tm_mday,
                      &tm_val.tm_hour, &tm_val.tm_min, &tm_val.tm_sec) == 6) {
                tm_val.tm_year -= 1900;
                tm_val.tm_mon -= 1;
                tm_val.tm_isdst = -1;
                SetFromTm(tm_val);
                return true;
            }
            break;
        }
        case DM_DATETIME_FORMAT_YYYYMMDD: {
            // 2024-01-01
            if (sscanf(str.c_str(), "%d-%d-%d", 
                      &tm_val.tm_year, &tm_val.tm_mon, &tm_val.tm_mday) == 3) {
                tm_val.tm_year -= 1900;
                tm_val.tm_mon -= 1;
                tm_val.tm_hour = 0;
                tm_val.tm_min = 0;
                tm_val.tm_sec = 0;
                tm_val.tm_isdst = -1;
                SetFromTm(tm_val);
                return true;
            }
            break;
        }
        case DM_DATETIME_FORMAT_TIMESTAMP: {
            time_t timestamp = static_cast<time_t>(std::stoll(str));
            SetFromTimestamp(timestamp);
            return true;
        }
        case DM_DATETIME_FORMAT_ISO8601: {
            // 简化的ISO8601解析: 2024-01-01T12:30:45Z
            char timeZone;
            if (sscanf(str.c_str(), "%d-%d-%dT%d:%d:%d%c", 
                      &tm_val.tm_year, &tm_val.tm_mon, &tm_val.tm_mday,
                      &tm_val.tm_hour, &tm_val.tm_min, &tm_val.tm_sec, &timeZone) == 7) {
                tm_val.tm_year -= 1900;
                tm_val.tm_mon -= 1;
                tm_val.tm_isdst = -1;
                SetFromTm(tm_val);
                return true;
            }
            break;
        }
        default:
            return false;
    }
    
    return false;
}

int CDMDateTimeImpl::GetYear() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_year + 1900;
}

int CDMDateTimeImpl::GetMonth() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_mon + 1;
}

int CDMDateTimeImpl::GetDay() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_mday;
}

int CDMDateTimeImpl::GetHour() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_hour;
}

int CDMDateTimeImpl::GetMinute() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_min;
}

int CDMDateTimeImpl::GetSecond() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_sec;
}

int CDMDateTimeImpl::GetWeekDay() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_wday;
}

int CDMDateTimeImpl::GetYearDay() const {
    if (!m_isValid) return 0;
    auto tm_val = GetLocalTm();
    return tm_val.tm_yday + 1;
}

time_t CDMDateTimeImpl::GetTimestamp() const {
    if (!m_isValid) return 0;
    return std::chrono::system_clock::to_time_t(m_timePoint);
}

std::string CDMDateTimeImpl::ToString(DMDateTimeFormat format) const {
    if (!m_isValid) return "";
    
    auto tm_val = GetLocalTm();
    std::ostringstream oss;
    
    switch (format) {
        case DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS:
            oss << std::setfill('0') << std::setw(4) << (tm_val.tm_year + 1900) << "-"
                << std::setw(2) << (tm_val.tm_mon + 1) << "-"
                << std::setw(2) << tm_val.tm_mday << " "
                << std::setw(2) << tm_val.tm_hour << ":"
                << std::setw(2) << tm_val.tm_min << ":"
                << std::setw(2) << tm_val.tm_sec;
            break;
            
        case DM_DATETIME_FORMAT_YYYYMMDD:
            oss << std::setfill('0') << std::setw(4) << (tm_val.tm_year + 1900) << "-"
                << std::setw(2) << (tm_val.tm_mon + 1) << "-"
                << std::setw(2) << tm_val.tm_mday;
            break;
            
        case DM_DATETIME_FORMAT_HHMMSS:
            oss << std::setfill('0') << std::setw(2) << tm_val.tm_hour << ":"
                << std::setw(2) << tm_val.tm_min << ":"
                << std::setw(2) << tm_val.tm_sec;
            break;
            
        case DM_DATETIME_FORMAT_TIMESTAMP:
            oss << GetTimestamp();
            break;
            
        case DM_DATETIME_FORMAT_ISO8601:
            oss << std::setfill('0') << std::setw(4) << (tm_val.tm_year + 1900) << "-"
                << std::setw(2) << (tm_val.tm_mon + 1) << "-"
                << std::setw(2) << tm_val.tm_mday << "T"
                << std::setw(2) << tm_val.tm_hour << ":"
                << std::setw(2) << tm_val.tm_min << ":"
                << std::setw(2) << tm_val.tm_sec << "Z";
            break;
            
        case DM_DATETIME_FORMAT_RFC2822: {
            const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            oss << weekdays[tm_val.tm_wday] << ", "
                << std::setfill('0') << std::setw(2) << tm_val.tm_mday << " "
                << months[tm_val.tm_mon] << " "
                << (tm_val.tm_year + 1900) << " "
                << std::setw(2) << tm_val.tm_hour << ":"
                << std::setw(2) << tm_val.tm_min << ":"
                << std::setw(2) << tm_val.tm_sec << " GMT";
            break;
        }
    }
    
    return oss.str();
}

void CDMDateTimeImpl::AddTime(int value, DMTimeUnit unit) {
    if (!m_isValid) return;
    
    switch (unit) {
        case DM_TIME_UNIT_SECOND:
            m_timePoint += std::chrono::seconds(value);
            break;
        case DM_TIME_UNIT_MINUTE:
            m_timePoint += std::chrono::minutes(value);
            break;
        case DM_TIME_UNIT_HOUR:
            m_timePoint += std::chrono::hours(value);
            break;
        case DM_TIME_UNIT_DAY:
            m_timePoint += std::chrono::hours(24 * value);
            break;
        case DM_TIME_UNIT_WEEK:
            m_timePoint += std::chrono::hours(24 * 7 * value);
            break;
        case DM_TIME_UNIT_MONTH: {
            auto tm_val = GetLocalTm();
            tm_val.tm_mon += value;
            while (tm_val.tm_mon >= 12) {
                tm_val.tm_mon -= 12;
                tm_val.tm_year++;
            }
            while (tm_val.tm_mon < 0) {
                tm_val.tm_mon += 12;
                tm_val.tm_year--;
            }
            SetFromTm(tm_val);
            break;
        }
        case DM_TIME_UNIT_YEAR: {
            auto tm_val = GetLocalTm();
            tm_val.tm_year += value;
            SetFromTm(tm_val);
            break;
        }
    }
}

void CDMDateTimeImpl::SubTime(int value, DMTimeUnit unit) {
    AddTime(-value, unit);
}

long long CDMDateTimeImpl::DiffTime(const IDMDateTime* other, DMTimeUnit unit) const {
    if (!m_isValid || !other || !other->IsValid()) return 0;
    
    auto otherTimestamp = other->GetTimestamp();
    auto thisTimestamp = GetTimestamp();
    auto diffSeconds = static_cast<long long>(thisTimestamp - otherTimestamp);
    
    switch (unit) {
        case DM_TIME_UNIT_SECOND:
            return diffSeconds;
        case DM_TIME_UNIT_MINUTE:
            return diffSeconds / 60;
        case DM_TIME_UNIT_HOUR:
            return diffSeconds / 3600;
        case DM_TIME_UNIT_DAY:
            return diffSeconds / (24 * 3600);
        case DM_TIME_UNIT_WEEK:
            return diffSeconds / (7 * 24 * 3600);
        case DM_TIME_UNIT_MONTH:
            // 近似计算，按30天计算
            return diffSeconds / (30 * 24 * 3600);
        case DM_TIME_UNIT_YEAR:
            // 近似计算，按365天计算
            return diffSeconds / (365 * 24 * 3600);
    }
    
    return 0;
}

bool CDMDateTimeImpl::IsEqual(const IDMDateTime* other) const {
    if (!other) return false;
    return GetTimestamp() == other->GetTimestamp();
}

bool CDMDateTimeImpl::IsLess(const IDMDateTime* other) const {
    if (!other) return false;
    return GetTimestamp() < other->GetTimestamp();
}

bool CDMDateTimeImpl::IsGreater(const IDMDateTime* other) const {
    if (!other) return false;
    return GetTimestamp() > other->GetTimestamp();
}

bool CDMDateTimeImpl::IsLessOrEqual(const IDMDateTime* other) const {
    if (!other) return false;
    return GetTimestamp() <= other->GetTimestamp();
}

bool CDMDateTimeImpl::IsGreaterOrEqual(const IDMDateTime* other) const {
    if (!other) return false;
    return GetTimestamp() >= other->GetTimestamp();
}

bool CDMDateTimeImpl::IsValid() const {
    return m_isValid;
}

bool CDMDateTimeImpl::IsLeapYear() const {
    if (!m_isValid) return false;
    return IsLeapYear(GetYear());
}

bool CDMDateTimeImpl::IsLeapYear(int year) const {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int CDMDateTimeImpl::GetDaysInMonth() const {
    if (!m_isValid) return 0;
    return GetDaysInMonth(GetYear(), GetMonth());
}

int CDMDateTimeImpl::GetDaysInMonth(int year, int month) const {
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month < 1 || month > 12) return 0;
    
    int days = daysInMonth[month - 1];
    if (month == 2 && IsLeapYear(year)) {
        days = 29;
    }
    
    return days;
}

void CDMDateTimeImpl::SetTimeZoneOffset(int offsetMinutes) {
    m_timezoneOffset = offsetMinutes;
}

int CDMDateTimeImpl::GetTimeZoneOffset() const {
    return m_timezoneOffset;
}

void CDMDateTimeImpl::ConvertToUTC() {
    if (!m_isValid) return;
    m_timePoint -= std::chrono::minutes(m_timezoneOffset);
}

void CDMDateTimeImpl::ConvertFromUTC() {
    if (!m_isValid) return;
    m_timePoint += std::chrono::minutes(m_timezoneOffset);
}

IDMDateTime* CDMDateTimeImpl::Clone() const {
    CDMDateTimeImpl* clone = new CDMDateTimeImpl();
    clone->m_timePoint = m_timePoint;
    clone->m_timezoneOffset = m_timezoneOffset;
    clone->m_isValid = m_isValid;
    return clone;
}

// 导出函数实现
extern "C" DMEXPORT_DLL IDMDateTime* DMAPI dmdatetimeGetModule() {
    return new CDMDateTimeImpl();
}

extern "C" DMEXPORT_DLL IDMDateTime* DMAPI dmdatetimeCreateFromTimestamp(time_t timestamp) {
    CDMDateTimeImpl* datetime = new CDMDateTimeImpl();
    datetime->SetFromTimestamp(timestamp);
    return datetime;
}

extern "C" DMEXPORT_DLL IDMDateTime* DMAPI dmdatetimeCreateFromString(const char* dateTimeStr, DMDateTimeFormat format) {
    if (!dateTimeStr) return nullptr;
    
    CDMDateTimeImpl* datetime = new CDMDateTimeImpl();
    datetime->SetFromString(std::string(dateTimeStr), format);
    
    if (!datetime->IsValid()) {
        datetime->Release();
        return nullptr;
    }
    
    return datetime;
}

extern "C" DMEXPORT_DLL IDMDateTime* DMAPI dmdatetimeCreateNow() {
    CDMDateTimeImpl* datetime = new CDMDateTimeImpl();
    datetime->SetToNow();
    return datetime;
}