
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

#ifndef __CDMDATETIME_H_INCLUDE__
#define __CDMDATETIME_H_INCLUDE__
#include "dmos.h" // dmos.h已经处理平台头文件, 以及相关宏定义
#include <string>
#include <ctime>

enum DMDateTimeFormat {
    DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS = 0,    // 2024-01-01 12:30:45
    DM_DATETIME_FORMAT_YYYYMMDD,               // 2024-01-01
    DM_DATETIME_FORMAT_HHMMSS,                 // 12:30:45
    DM_DATETIME_FORMAT_TIMESTAMP,              // 1704096645
    DM_DATETIME_FORMAT_ISO8601,                // 2024-01-01T12:30:45Z
    DM_DATETIME_FORMAT_RFC2822                 // Mon, 01 Jan 2024 12:30:45 GMT
};

enum DMTimeUnit {
    DM_TIME_UNIT_SECOND = 0,
    DM_TIME_UNIT_MINUTE,
    DM_TIME_UNIT_HOUR,
    DM_TIME_UNIT_DAY,
    DM_TIME_UNIT_WEEK,
    DM_TIME_UNIT_MONTH,
    DM_TIME_UNIT_YEAR
};

class ICDMDateTime
{
public:
    virtual ~ICDMDateTime() {}
    virtual void DMAPI Release(void) = 0;
    
    // 基础设置和获取
    virtual void DMAPI SetDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) = 0;
    virtual void DMAPI SetFromTimestamp(time_t timestamp) = 0;
    virtual void DMAPI SetFromString(const std::string& dateTimeStr, DMDateTimeFormat format = DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS) = 0;
    virtual void DMAPI SetToNow() = 0;
    
    // 获取时间信息
    virtual int DMAPI GetYear() const = 0;
    virtual int DMAPI GetMonth() const = 0;
    virtual int DMAPI GetDay() const = 0;
    virtual int DMAPI GetHour() const = 0;
    virtual int DMAPI GetMinute() const = 0;
    virtual int DMAPI GetSecond() const = 0;
    virtual int DMAPI GetWeekDay() const = 0;  // 0-6 (Sunday-Saturday)
    virtual int DMAPI GetYearDay() const = 0;  // 1-366
    
    // 时间戳和字符串转换
    virtual time_t DMAPI GetTimestamp() const = 0;
    virtual std::string DMAPI ToString(DMDateTimeFormat format = DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS) const = 0;
    
    // 时间运算
    virtual void DMAPI AddTime(int value, DMTimeUnit unit) = 0;
    virtual void DMAPI SubTime(int value, DMTimeUnit unit) = 0;
    virtual long long DMAPI DiffTime(const ICDMDateTime* other, DMTimeUnit unit) const = 0;
    
    // 时间比较
    virtual bool DMAPI IsEqual(const ICDMDateTime* other) const = 0;
    virtual bool DMAPI IsLess(const ICDMDateTime* other) const = 0;
    virtual bool DMAPI IsGreater(const ICDMDateTime* other) const = 0;
    virtual bool DMAPI IsLessOrEqual(const ICDMDateTime* other) const = 0;
    virtual bool DMAPI IsGreaterOrEqual(const ICDMDateTime* other) const = 0;
    
    // 日期验证和工具
    virtual bool DMAPI IsValid() const = 0;
    virtual bool DMAPI IsLeapYear() const = 0;
    virtual bool DMAPI IsLeapYear(int year) const = 0;
    virtual int DMAPI GetDaysInMonth() const = 0;
    virtual int DMAPI GetDaysInMonth(int year, int month) const = 0;
    
    // 时区操作
    virtual void DMAPI SetTimeZoneOffset(int offsetMinutes) = 0;
    virtual int DMAPI GetTimeZoneOffset() const = 0;
    virtual void DMAPI ConvertToUTC() = 0;
    virtual void DMAPI ConvertFromUTC() = 0;
    
    // 克隆
    virtual ICDMDateTime* DMAPI Clone() const = 0;
};

extern "C" DMEXPORT_DLL ICDMDateTime* DMAPI cdmdatetimeGetModule();
typedef ICDMDateTime* (DMAPI* PFN_cdmdatetimeGetModule)();

extern "C" DMEXPORT_DLL ICDMDateTime* DMAPI cdmdatetimeCreateFromTimestamp(time_t timestamp);
extern "C" DMEXPORT_DLL ICDMDateTime* DMAPI cdmdatetimeCreateFromString(const char* dateTimeStr, DMDateTimeFormat format);
extern "C" DMEXPORT_DLL ICDMDateTime* DMAPI cdmdatetimeCreateNow();

#endif // __DMDATETIME_H_INCLUDE__