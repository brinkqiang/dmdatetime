
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

#ifndef __CDMDATETIME_IMPL_H_INCLUDE__
#define __CDMDATETIME_IMPL_H_INCLUDE__

#include "dmdatetime.h"
#include <chrono>

class CDMDateTimeImpl : public IDMDateTime
{
private:
    std::chrono::system_clock::time_point m_timePoint;
    int m_timezoneOffset; // 时区偏移，单位为分钟
    bool m_isValid;
    
    // 内部辅助函数
    std::tm GetLocalTm() const;
    void SetFromTm(const std::tm& tm);
    bool IsValidDate(int year, int month, int day) const;
    bool IsValidTime(int hour, int minute, int second) const;
    std::string FormatWithPattern(const std::string& pattern) const;
    bool ParseFromString(const std::string& str, DMDateTimeFormat format);
    
public:
    CDMDateTimeImpl();
    virtual ~CDMDateTimeImpl() {}
    
    virtual void DMAPI Release(void) override;
    
    // 基础设置和获取
    virtual void DMAPI SetDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) override;
    virtual void DMAPI SetFromTimestamp(time_t timestamp) override;
    virtual void DMAPI SetFromString(const std::string& dateTimeStr, DMDateTimeFormat format = DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS) override;
    virtual void DMAPI SetToNow() override;
    
    // 获取时间信息
    virtual int DMAPI GetYear() const override;
    virtual int DMAPI GetMonth() const override;
    virtual int DMAPI GetDay() const override;
    virtual int DMAPI GetHour() const override;
    virtual int DMAPI GetMinute() const override;
    virtual int DMAPI GetSecond() const override;
    virtual int DMAPI GetWeekDay() const override;
    virtual int DMAPI GetYearDay() const override;
    
    // 时间戳和字符串转换
    virtual time_t DMAPI GetTimestamp() const override;
    virtual std::string DMAPI ToString(DMDateTimeFormat format = DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS) const override;
    
    // 时间运算
    virtual void DMAPI AddTime(int value, DMTimeUnit unit) override;
    virtual void DMAPI SubTime(int value, DMTimeUnit unit) override;
    virtual long long DMAPI DiffTime(const IDMDateTime* other, DMTimeUnit unit) const override;
    
    // 时间比较
    virtual bool DMAPI IsEqual(const IDMDateTime* other) const override;
    virtual bool DMAPI IsLess(const IDMDateTime* other) const override;
    virtual bool DMAPI IsGreater(const IDMDateTime* other) const override;
    virtual bool DMAPI IsLessOrEqual(const IDMDateTime* other) const override;
    virtual bool DMAPI IsGreaterOrEqual(const IDMDateTime* other) const override;
    
    // 日期验证和工具
    virtual bool DMAPI IsValid() const override;
    virtual bool DMAPI IsLeapYear() const override;
    virtual bool DMAPI IsLeapYear(int year) const override;
    virtual int DMAPI GetDaysInMonth() const override;
    virtual int DMAPI GetDaysInMonth(int year, int month) const override;
    
    // 时区操作
    virtual void DMAPI SetTimeZoneOffset(int offsetMinutes) override;
    virtual int DMAPI GetTimeZoneOffset() const override;
    virtual void DMAPI ConvertToUTC() override;
    virtual void DMAPI ConvertFromUTC() override;
    
    // 克隆
    virtual IDMDateTime* DMAPI Clone() const override;
};

#endif