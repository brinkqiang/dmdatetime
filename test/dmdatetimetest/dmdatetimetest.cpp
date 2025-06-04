#include "dmdatetime.h"
#include <string>
#include <vector>
#include <numeric>
#include "gtest.h"
#include "dmformat.h"
#include "dmfix_win_utf8.h"

class env_dmdatetime
{
public:
    void init() {}
    void uninit() {}
};

class CDMDateTimeUsageTest : public ::testing::Test {
public:
    virtual void SetUp()
    {
        env.init();

        dt_ref = CDMDateTime(2024, 12, 25, 15, 30, 45);
        dt_ref_midnight = CDMDateTime(2024, 12, 25);
        // dt_ts_ref; // For timestamp 1703512245 (2023-12-25 13:50:45 UTC)
        // Initialize dt_ts_ref if it's used elsewhere, or remove if not.
        // For now, assuming it might be used for UTC tests, let's set it up.
        // The original comment indicated 1703512245 is 2023-12-25 13:50:45 UTC
        dt_ts_ref = CDMDateTime::FromTimestamp(1703512245L);
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmdatetime env;
    CDMDateTime dt_ref;
    CDMDateTime dt_ref_midnight;
    CDMDateTime dt_ts_ref;

};

TEST_F(CDMDateTimeUsageTest, CreationMethods) {
    CDMDateTime now = CDMDateTime::Now();
    EXPECT_GT(now.GetYear(), 2020);

    CDMDateTime dt1_parse = CDMDateTime::Parse("2024-12-25 15:30:45");
    EXPECT_EQ(dt_ref, dt1_parse);

    CDMDateTime dt2_parse_format = CDMDateTime::Parse("2024/12/25 15:30:45", "%d/%d/%d %d:%d:%d");
    EXPECT_EQ(dt_ref, dt2_parse_format);

    CDMDateTime dt3_components(2024, 12, 25, 15, 30, 45);
    EXPECT_EQ(dt_ref, dt3_components);

    CDMDateTime dt4_date_only(2024, 12, 25);
    EXPECT_EQ(dt_ref_midnight.GetYear(), dt4_date_only.GetYear());
    EXPECT_EQ(dt_ref_midnight.GetMonth(), dt4_date_only.GetMonth());
    EXPECT_EQ(dt_ref_midnight.GetDay(), dt4_date_only.GetDay());
    EXPECT_EQ(0, dt4_date_only.GetHour());
    EXPECT_EQ(0, dt4_date_only.GetMinute());
    EXPECT_EQ(0, dt4_date_only.GetSecond());

    time_t test_timestamp_sec = 1703512245L; // 2023-12-25 13:50:45 UTC
    CDMDateTime dt5_from_ts = CDMDateTime::FromTimestamp(test_timestamp_sec);
    // ToUTC() is a no-op now in the header, so the timestamp of dt5_from_ts (which is constructed from a UTC epoch)
    // should directly match test_timestamp_sec.
    EXPECT_EQ(test_timestamp_sec, dt5_from_ts.GetTimestamp());
}

TEST_F(CDMDateTimeUsageTest, GetComponentValues) {
    EXPECT_EQ(2024, dt_ref.GetYear());
    EXPECT_EQ(12, dt_ref.GetMonth());
    EXPECT_EQ(25, dt_ref.GetDay());
    EXPECT_EQ(15, dt_ref.GetHour());
    EXPECT_EQ(30, dt_ref.GetMinute());
    EXPECT_EQ(45, dt_ref.GetSecond());
    // Milliseconds removed
    EXPECT_EQ(3, dt_ref.GetDayOfWeek()); // 2024-12-25 is Wednesday (0=Sun, 1=Mon, ..., 3=Wed)
    EXPECT_EQ(360, dt_ref.GetDayOfYear()); // 2024 is a leap year. 31+29+31+30+31+30+31+31+30+31+30+25
}

TEST_F(CDMDateTimeUsageTest, TimeArithmetic) {
    CDMDateTime tomorrow = dt_ref.AddDays(1);
    EXPECT_EQ(2024, tomorrow.GetYear());
    EXPECT_EQ(12, tomorrow.GetMonth());
    EXPECT_EQ(26, tomorrow.GetDay());
    EXPECT_EQ(dt_ref.GetHour(), tomorrow.GetHour());

    CDMDateTime nextWeek = dt_ref.AddDays(7);
    EXPECT_EQ(2025, nextWeek.GetYear()); // 25 + 7 = 32, so Jan 1st
    EXPECT_EQ(1, nextWeek.GetMonth());
    EXPECT_EQ(1, nextWeek.GetDay());

    CDMDateTime nextMonth = dt_ref.AddMonths(1);
    EXPECT_EQ(2025, nextMonth.GetYear());
    EXPECT_EQ(1, nextMonth.GetMonth());
    EXPECT_EQ(25, nextMonth.GetDay());

    CDMDateTime nextYear = dt_ref.AddYears(1);
    EXPECT_EQ(2025, nextYear.GetYear());
    EXPECT_EQ(12, nextYear.GetMonth());
    EXPECT_EQ(25, nextYear.GetDay());

    CDMDateTime later = dt_ref.AddHours(2).AddMinutes(30).AddSeconds(15); // 15:30:45 + 2h30m15s = 18:01:00
    EXPECT_EQ(18, later.GetHour());
    EXPECT_EQ(1, later.GetMinute());
    EXPECT_EQ(0, later.GetSecond());

    CDMDateTime yesterday = dt_ref.AddDays(-1);
    EXPECT_EQ(2024, yesterday.GetYear());
    EXPECT_EQ(12, yesterday.GetMonth());
    EXPECT_EQ(24, yesterday.GetDay());

    CDMTimeSpan diff = dt_ref.Subtract(yesterday);
    EXPECT_DOUBLE_EQ(1.0, diff.GetTotalDays());
    EXPECT_DOUBLE_EQ(24.0, diff.GetTotalHours());
    EXPECT_DOUBLE_EQ(1440.0, diff.GetTotalMinutes());
    EXPECT_DOUBLE_EQ(86400.0, diff.GetTotalSeconds());
    // Milliseconds removed
}

TEST_F(CDMDateTimeUsageTest, ComparisonOperators) {
    CDMDateTime yesterday = dt_ref.AddDays(-1);
    CDMDateTime dt_ref_copy = dt_ref;

    EXPECT_TRUE(dt_ref > yesterday);
    EXPECT_FALSE(yesterday > dt_ref);
    EXPECT_TRUE(dt_ref >= yesterday);
    EXPECT_TRUE(dt_ref >= dt_ref_copy);

    EXPECT_TRUE(yesterday < dt_ref);
    EXPECT_FALSE(dt_ref < yesterday);
    EXPECT_TRUE(yesterday <= dt_ref);
    EXPECT_TRUE(dt_ref <= dt_ref_copy);

    EXPECT_TRUE(dt_ref == dt_ref_copy);
    EXPECT_FALSE(dt_ref == yesterday);

    EXPECT_TRUE(dt_ref != yesterday);
    EXPECT_FALSE(dt_ref != dt_ref_copy);
}

TEST_F(CDMDateTimeUsageTest, SpecialDateOperations) {
    CDMDateTime startOfDay = dt_ref.GetStartOfDay();
    EXPECT_EQ(2024, startOfDay.GetYear());
    EXPECT_EQ(12, startOfDay.GetMonth());
    EXPECT_EQ(25, startOfDay.GetDay());
    EXPECT_EQ(0, startOfDay.GetHour());
    EXPECT_EQ(0, startOfDay.GetMinute());
    EXPECT_EQ(0, startOfDay.GetSecond());
    // Milliseconds removed

    CDMDateTime endOfDay = dt_ref.GetEndOfDay();
    EXPECT_EQ(2024, endOfDay.GetYear());
    EXPECT_EQ(12, endOfDay.GetMonth());
    EXPECT_EQ(25, endOfDay.GetDay());
    EXPECT_EQ(23, endOfDay.GetHour());
    EXPECT_EQ(59, endOfDay.GetMinute());
    EXPECT_EQ(59, endOfDay.GetSecond());
    // Milliseconds removed

    CDMDateTime startOfMonth = dt_ref.GetStartOfMonth();
    EXPECT_EQ(2024, startOfMonth.GetYear());
    EXPECT_EQ(12, startOfMonth.GetMonth());
    EXPECT_EQ(1, startOfMonth.GetDay());
    EXPECT_EQ(0, startOfMonth.GetHour());

    CDMDateTime endOfMonth = dt_ref.GetEndOfMonth();
    EXPECT_EQ(2024, endOfMonth.GetYear());
    EXPECT_EQ(12, endOfMonth.GetMonth());
    EXPECT_EQ(31, endOfMonth.GetDay());
    EXPECT_EQ(23, endOfMonth.GetHour());
    EXPECT_EQ(59, endOfMonth.GetMinute());
    EXPECT_EQ(59, endOfMonth.GetSecond());

    CDMDateTime startOfYear = dt_ref.GetStartOfYear();
    EXPECT_EQ(2024, startOfYear.GetYear());
    EXPECT_EQ(1, startOfYear.GetMonth());
    EXPECT_EQ(1, startOfYear.GetDay());
    EXPECT_EQ(0, startOfYear.GetHour());
}

TEST_F(CDMDateTimeUsageTest, TimeZoneOperations) {
    // ToUTC and ToLocal are now no-ops in the provided header, returning *this.
    // This test needs to be re-evaluated based on that behavior.
    // If dt_ref is created with local components, it represents local time.
    // Its GetTimestamp() will give the UTC epoch seconds for that local time.
    CDMDateTime utcTimeEquivalent = CDMDateTime::FromTimestamp(dt_ref.GetTimestamp()); // This is effectively dt_ref's UTC equivalent
    CDMDateTime localTime = utcTimeEquivalent; // ToLocal is no-op

    // dt_ref is local: 2024-12-25 15:30:45
    // localTime (which is utcTimeEquivalent) will represent 2024-12-25 15:30:45 (if current TZ is UTC)
    // or the equivalent UTC time shown in local TZ.
    // The direct comparison EXPECT_EQ(dt_ref, localTime) might only pass if the system's TZ is UTC,
    // because dt_ref is initialized with components interpreted as local,
    // while localTime is derived from dt_ref's UTC timestamp.
    // If CDMDateTime always normalizes to a specific representation (e.g. UTC internally,
    // and to_tm_local() converts for Getters), this could be more consistent.
    // Given the header uses system_clock and to_time_t then localtime_r/gmtime_r,
    // the interpretation is: time_point_ is UTC. Getters convert to local.
    // So, dt_ref constructed from components will have its time_point_ reflect the UTC equivalent of those local components.
    // utcTimeEquivalent is dt_ref itself because ToUTC is a no-op.
    // localTime is also dt_ref because ToLocal is a no-op.
    // Therefore, this test should simplify or be rethought.
    EXPECT_EQ(dt_ref.GetTimestamp(), localTime.GetTimestamp());


    CDMDateTime specificTZ = dt_ref.ToTimeZone(8); // ToTimeZone is also a no-op.
    EXPECT_EQ(dt_ref.GetTimestamp(), specificTZ.GetTimestamp()); // Timestamp should be unchanged as it's a no-op
    EXPECT_NO_THROW(dt_ref.ToTimeZone(8));
    EXPECT_NO_THROW(dt_ref.ToTimeZone(-5));
}

TEST_F(CDMDateTimeUsageTest, ValidationAndUtilityFunctions) {
    EXPECT_TRUE(dt_ref.IsLeapYear()); // 2024 is a leap year
    CDMDateTime nonLeapYear(2023, 1, 1);
    EXPECT_FALSE(nonLeapYear.IsLeapYear());

    EXPECT_TRUE(dt_ref.IsWeekday()); // 2024-12-25 is Wednesday
    EXPECT_FALSE(dt_ref.IsWeekend());

    CDMDateTime saturday(2024, 12, 28); // Saturday
    EXPECT_TRUE(saturday.IsWeekend());
    EXPECT_FALSE(saturday.IsWeekday());

    CDMDateTime sunday(2024, 12, 29); // Sunday
    EXPECT_TRUE(sunday.IsWeekend());
    EXPECT_FALSE(sunday.IsWeekday());

    time_t timestamp_sec = dt_ref.GetTimestamp();
    CDMDateTime from_ts_sec = CDMDateTime::FromTimestamp(timestamp_sec);
    EXPECT_EQ(dt_ref.GetTimestamp(), from_ts_sec.GetTimestamp()); // Comparing raw UTC timestamps
    // Milliseconds tests removed
}

TEST_F(CDMDateTimeUsageTest, ChainedOperations) {
    CDMDateTime start_chain(2023, 1, 10, 10, 30, 0);
    CDMDateTime result = start_chain
        .AddYears(1)     // 2024-01-10 10:30:00
        .AddMonths(2)    // 2024-03-10 10:30:00
        .AddDays(3)      // 2024-03-13 10:30:00
        .GetStartOfDay(); // 2024-03-13 00:00:00

    EXPECT_EQ(2024, result.GetYear());
    EXPECT_EQ(3, result.GetMonth());
    EXPECT_EQ(13, result.GetDay());
    EXPECT_EQ(0, result.GetHour());
    EXPECT_EQ(0, result.GetMinute());
    EXPECT_EQ(0, result.GetSecond());
}

TEST_F(CDMDateTimeUsageTest, StaticUtilityMethods) {
    CDMDateTime today = CDMDateTime::Today();
    EXPECT_EQ(0, today.GetHour());
    EXPECT_EQ(0, today.GetMinute());
    EXPECT_EQ(0, today.GetSecond());
    // Milliseconds removed

    CDMDateTime now = CDMDateTime::Now();
    EXPECT_EQ(now.GetYear(), today.GetYear());
    EXPECT_EQ(now.GetMonth(), today.GetMonth());
    EXPECT_EQ(now.GetDay(), today.GetDay());

    CDMDateTime minValue = CDMDateTime::MinValue();
    CDMDateTime maxValue = CDMDateTime::MaxValue();
    EXPECT_TRUE(minValue < dt_ref);
    EXPECT_TRUE(maxValue > dt_ref);
    EXPECT_TRUE(minValue < maxValue);

    // Check against the values defined in CDMDateTime.h
    time_t current_time_t;
    std::time(&current_time_t);
    if (sizeof(time_t) > 4) { // Assuming 64-bit time_t corresponds to year 3000 in MaxValue
        EXPECT_EQ(3000, maxValue.GetYear());
    }
    else { // Assuming 32-bit time_t corresponds to year 2038
        EXPECT_EQ(2038, maxValue.GetYear());
    }
    EXPECT_EQ(1970, minValue.GetYear());
}

TEST_F(CDMDateTimeUsageTest, RangeChecking) {
    CDMDateTime start_range = CDMDateTime::Parse("2024-01-01 00:00:00");
    CDMDateTime end_range = CDMDateTime::Parse("2024-12-31 23:59:59");

    EXPECT_TRUE(dt_ref.IsBetween(start_range, end_range)); // dt_ref is 2024-12-25

    CDMDateTime before_range = CDMDateTime::Parse("2023-12-31 23:59:59");
    EXPECT_FALSE(before_range.IsBetween(start_range, end_range));

    CDMDateTime after_range = CDMDateTime::Parse("2025-01-01 00:00:00");
    EXPECT_FALSE(after_range.IsBetween(start_range, end_range));

    EXPECT_TRUE(start_range.IsBetween(start_range, end_range)); // Inclusive start
    EXPECT_TRUE(end_range.IsBetween(start_range, end_range));   // Inclusive end

    CDMDateTime mid_range(2024, 6, 15);
    EXPECT_TRUE(mid_range.IsBetween(start_range, end_range));
}

TEST_F(CDMDateTimeUsageTest, LunarSupport) {
    std::string lunar_str = dt_ref.ToLunarString();
    EXPECT_FALSE(lunar_str.empty());
    EXPECT_EQ("农历支持未实现 (Lunar support not implemented)", lunar_str);
}

TEST_F(CDMDateTimeUsageTest, FormatConstants) {
    EXPECT_EQ(dt_ref.ToString(), dt_ref.ToString(CDMDateTime::TO_STRING_STANDARD));
    EXPECT_EQ("2024-12-25", dt_ref.ToString(CDMDateTime::TO_STRING_SHORT_DATE));

    std::string cn_standard_str = dt_ref.ToString(CDMDateTime::TO_STRING_STANDARD_CN);

    fmt::print("{}\n", cn_standard_str);
    EXPECT_NE(std::string::npos, cn_standard_str.find("2024年"));
    EXPECT_NE(std::string::npos, cn_standard_str.find("12月"));
    EXPECT_NE(std::string::npos, cn_standard_str.find("25日"));
    EXPECT_NE(std::string::npos, cn_standard_str.find("15时"));
    EXPECT_NE(std::string::npos, cn_standard_str.find("30分"));
    EXPECT_NE(std::string::npos, cn_standard_str.find("45秒"));

    std::string cn_short_date_str = dt_ref.ToString(CDMDateTime::TO_STRING_SHORT_DATE_CN);
    EXPECT_NE(std::string::npos, cn_short_date_str.find("2024年"));
    EXPECT_NE(std::string::npos, cn_short_date_str.find("12月"));
    EXPECT_NE(std::string::npos, cn_short_date_str.find("25日"));
    EXPECT_EQ(std::string::npos, cn_short_date_str.find("时")); // Should not contain time
}


class CDMDateTimePracticalTest : public ::testing::Test {
};

TEST_F(CDMDateTimePracticalTest, LogFileNameGeneration) {
    CDMDateTime fixed_now(2024, 7, 15, 10, 30, 0);
    std::string logFileName = "log_" + fixed_now.ToString("%04d_%02d_%02d_%02d_%02d_%02d") + ".txt";
    EXPECT_EQ("log_2024_07_15_10_30_00.txt", logFileName);
}

TEST_F(CDMDateTimePracticalTest, CalculateAge) {
    CDMDateTime birthday(1990, 5, 15);
    CDMDateTime current_fixed_date(2024, 6, 4); // Fixed date for test consistency

    CDMTimeSpan age_span = current_fixed_date.Subtract(birthday);
    int years_calculated_approx = static_cast<int>(age_span.GetTotalDays() / 365.25);
    EXPECT_EQ(34, years_calculated_approx);

    int precise_years = current_fixed_date.GetYear() - birthday.GetYear();
    if (current_fixed_date.GetMonth() < birthday.GetMonth() ||
        (current_fixed_date.GetMonth() == birthday.GetMonth() && current_fixed_date.GetDay() < birthday.GetDay())) {
        precise_years--;
    }
    EXPECT_EQ(precise_years, years_calculated_approx);

    CDMDateTime birthday_edge(2000, 1, 1);
    CDMDateTime current_edge(2023, 12, 31);
    CDMTimeSpan age_span_edge = current_edge.Subtract(birthday_edge);
    EXPECT_EQ(23, static_cast<int>(age_span_edge.GetTotalDays() / 365.25));
}

TEST_F(CDMDateTimePracticalTest, CalculateWorkdays) {
    CDMDateTime startDate = CDMDateTime::Parse("2024-01-01"); // Monday
    CDMDateTime endDate = CDMDateTime::Parse("2024-01-07");   // Sunday
    int workdays = 0;
    for (CDMDateTime date = startDate; date <= endDate; date = date.AddDays(1)) {
        if (date.IsWeekday()) {
            workdays++;
        }
    }
    EXPECT_EQ(5, workdays);

    CDMDateTime startDate2 = CDMDateTime::Parse("2024-12-23"); // Monday
    CDMDateTime endDate2 = CDMDateTime::Parse("2024-12-29");   // Sunday
    int workdays2 = 0;
    for (CDMDateTime date = startDate2; date <= endDate2; date = date.AddDays(1)) {
        if (date.IsWeekday()) {
            workdays2++;
        }
    }
    EXPECT_EQ(5, workdays2);
}

TEST_F(CDMDateTimePracticalTest, OperationTimeCalculation) {
    CDMDateTime startTime(2024, 1, 1, 10, 0, 0); // Millisecond argument removed
    CDMDateTime endTime(2024, 1, 1, 10, 0, 1); // Millisecond argument removed, was 1 sec 500ms

    CDMTimeSpan elapsed = endTime.Subtract(startTime);
    // Milliseconds test removed. Original was 1500ms. Now it's 1s.
    auto totalSeconds = elapsed.GetTotalSeconds();
    EXPECT_DOUBLE_EQ(1, totalSeconds);

    CDMDateTime startTime2(2024, 1, 1, 10, 0, 0); // Millisecond argument removed
    CDMDateTime endTime2(2024, 1, 1, 10, 1, 30); // Millisecond argument removed, was 1m 30s 250ms
    CDMTimeSpan elapsed2 = endTime2.Subtract(startTime2); // Now 1m 30s = 90s
    // Milliseconds test removed. Original was 90250ms.
    EXPECT_DOUBLE_EQ(90, elapsed2.GetTotalSeconds());
}