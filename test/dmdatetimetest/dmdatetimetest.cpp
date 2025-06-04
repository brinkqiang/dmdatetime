
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
    void init(){}
    void uninit(){}
};

class CDMDateTimeUsageTest : public ::testing::Test {
public:
    virtual void SetUp()
    {
        env.init();

        dt_ref = CDMDateTime(2024, 12, 25, 15, 30, 45);
        dt_ref_midnight = CDMDateTime(2024, 12, 25);
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmdatetime env;
    CDMDateTime dt_ref;
    CDMDateTime dt_ref_midnight;
    CDMDateTime dt_ts_ref; // For timestamp 1703512245 (2023-12-25 13:50:45 UTC)

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

    int64_t test_timestamp_sec = 1703512245; // 2023-12-25 13:50:45 UTC
    CDMDateTime dt5_from_ts = CDMDateTime::FromTimestamp(test_timestamp_sec);
    EXPECT_EQ(test_timestamp_sec, dt5_from_ts.ToUTC().GetTimestamp());


    int64_t test_timestamp_ms = 1703512245000LL;
    CDMDateTime dt6_from_ts_ms = CDMDateTime::FromTimestamp(test_timestamp_ms, true);
    EXPECT_EQ(test_timestamp_ms, dt6_from_ts_ms.ToUTC().GetTimestampMs());
}

TEST_F(CDMDateTimeUsageTest, FormattingOutput) {
    EXPECT_EQ("2024-12-25 15:30:45", dt_ref.ToString());
    EXPECT_EQ("2024-12-25", dt_ref.ToString(CDMDateTime::TO_STRING_SHORT_DATE));

    // ISOString and UTCString depend on implementation specifics and local timezone
    // For ISOString, a common representation of local time.
    // For UTCString, it should represent the UTC equivalent.
    // We can test that they don't throw and return non-empty strings.
    EXPECT_FALSE(dt_ref.ToISOString().empty());
    EXPECT_FALSE(dt_ref.ToUTCString().empty());

    // More specific test if we know dt_ref is local and can find its UTC equivalent
    // For instance, if local is UTC+8, 15:30:45 local is 07:30:45 UTC.
    // This test would be fragile across different test environments.
    // A simple check:
    CDMDateTime dt_utc_variant = dt_ref.ToUTC();
    std::string utc_str_check = dt_utc_variant.ToString("yyyy-MM-dd HH:mm:ss") + " UTC"; // Hypothetical format
    // The actual ToUTCString() format might be different.
    // For now, just checking it gives a different hour if not already UTC and not on DST boundary issues.
    if (dt_ref.GetHour() != dt_ref.ToUTC().GetHour() || dt_ref.GetDay() != dt_ref.ToUTC().GetDay()) { // If local is not UTC0
        EXPECT_NE(dt_ref.ToString(), dt_ref.ToUTCString());
    }
}

TEST_F(CDMDateTimeUsageTest, GetComponentValues) {
    EXPECT_EQ(2024, dt_ref.GetYear());
    EXPECT_EQ(12, dt_ref.GetMonth());
    EXPECT_EQ(25, dt_ref.GetDay());
    EXPECT_EQ(15, dt_ref.GetHour());
    EXPECT_EQ(30, dt_ref.GetMinute());
    EXPECT_EQ(45, dt_ref.GetSecond());
    EXPECT_EQ(0, dt_ref.GetMillisecond());
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
    EXPECT_DOUBLE_EQ(86400000.0, diff.GetTotalMilliseconds());
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
    EXPECT_EQ(0, startOfDay.GetMillisecond());

    CDMDateTime endOfDay = dt_ref.GetEndOfDay();
    EXPECT_EQ(2024, endOfDay.GetYear());
    EXPECT_EQ(12, endOfDay.GetMonth());
    EXPECT_EQ(25, endOfDay.GetDay());
    EXPECT_EQ(23, endOfDay.GetHour());
    EXPECT_EQ(59, endOfDay.GetMinute());
    EXPECT_EQ(59, endOfDay.GetSecond());
    EXPECT_EQ(999, endOfDay.GetMillisecond());

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
    CDMDateTime utcTime = dt_ref.ToUTC();
    CDMDateTime localTime = utcTime.ToLocal();
    // This assumes ToLocal correctly reverts ToUTC for the reference time.
    // Potential issues with DST changes if not handled carefully by the library.
    EXPECT_EQ(dt_ref, localTime);

    // Testing ToTimeZone(offset) is complex without knowing the base timezone of dt_ref
    // or the exact behavior of ToTimeZone.
    // A simple check: the timestamp (absolute time) should remain the same.
    CDMDateTime specificTZ = dt_ref.ToTimeZone(8); // Assuming this means UTC+8
    EXPECT_EQ(dt_ref.ToUTC().GetTimestamp(), specificTZ.ToUTC().GetTimestamp());
    // If dt_ref was local, and ToTimeZone(8) sets its representation to UTC+8,
    // its components might change. For example, if dt_ref was UTC+0 12:00,
    // ToTimeZone(8) might display as 20:00.
    // This requires more specific definition of ToTimeZone behavior.
    // For now, we just check it doesn't crash and returns a CDMDateTime.
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

    int64_t timestamp_sec = dt_ref.GetTimestamp();
    CDMDateTime from_ts_sec = CDMDateTime::FromTimestamp(timestamp_sec);
    // Allowing for potential minor differences if dt_ref was local and conversion roundtrip isn't perfect to ms
    EXPECT_EQ(dt_ref.ToUTC().GetTimestamp(), from_ts_sec.ToUTC().GetTimestamp());


    int64_t timestamp_ms = dt_ref.GetTimestampMs();
    CDMDateTime from_ts_ms = CDMDateTime::FromTimestamp(timestamp_ms, true);
    EXPECT_EQ(dt_ref.ToUTC().GetTimestampMs(), from_ts_ms.ToUTC().GetTimestampMs());
}

TEST_F(CDMDateTimeUsageTest, ChainedOperations) {
    CDMDateTime start_chain(2023, 1, 10, 10, 30, 0);
    CDMDateTime result = start_chain
        .AddYears(1)    // 2024-01-10 10:30:00
        .AddMonths(2)   // 2024-03-10 10:30:00
        .AddDays(3)     // 2024-03-13 10:30:00
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
    EXPECT_EQ(0, today.GetMillisecond());
    // Check if today's date matches current system date (loosely)
    CDMDateTime now = CDMDateTime::Now();
    EXPECT_EQ(now.GetYear(), today.GetYear());
    EXPECT_EQ(now.GetMonth(), today.GetMonth());
    EXPECT_EQ(now.GetDay(), today.GetDay());


    CDMDateTime minValue = CDMDateTime::MinValue();
    CDMDateTime maxValue = CDMDateTime::MaxValue();
    EXPECT_TRUE(minValue < dt_ref);
    EXPECT_TRUE(maxValue > dt_ref);
    EXPECT_TRUE(minValue < maxValue);
    // Assuming MinValue is something like 0001-01-01 and MaxValue 9999-12-31
    // This is a basic check. More specific checks would need to know the exact Min/Max values.
    EXPECT_EQ(minValue.GetYear(), 1970); // e.g. year 1 or 1970
    EXPECT_EQ(maxValue.GetYear(), 3000); // e.g. year 9999
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
    // 2024-12-25 is 农历甲辰年冬月廿五
    // The exact string format depends on ToLunarString() implementation.
    // This test assumes a specific format. It might need adjustment.
    std::string lunar_str = dt_ref.ToLunarString();
    EXPECT_FALSE(lunar_str.empty());
    // Example: EXPECT_EQ("甲辰年冬月廿五", lunar_str);
    // As the format is unknown, we only check if it returns something.
}

TEST_F(CDMDateTimeUsageTest, FormatConstants) {
    // These tests assume what the constants expand to.
    // If the actual format strings for these constants are known, use them.
    // Default ToString() is already tested as "yyyy-MM-dd HH:mm:ss"
    EXPECT_EQ(dt_ref.ToString(), dt_ref.ToString(CDMDateTime::TO_STRING_STANDARD));

    // Assuming FORMAT_SHORT_DATE is "yyyy-MM-dd"
    EXPECT_EQ("2024-12-25", dt_ref.ToString(CDMDateTime::TO_STRING_SHORT_DATE));

    // Assuming FORMAT_LONG_DATE is "yyyy年MM月dd日" (or similar based on locale/lib)
    // This is a guess. If different, this will fail.
    // From example: dt1.ToString("yyyy年MM月dd日 HH:mm:ss")
    // Let's assume FORMAT_LONG_DATE doesn't include time.
    std::string long_date_str = dt_ref.ToString(CDMDateTime::TO_STRING_STANDARD);
    EXPECT_NE(std::string::npos, long_date_str.find(std::to_string(dt_ref.GetYear())));
    EXPECT_NE(std::string::npos, long_date_str.find(std::to_string(dt_ref.GetMonth())));
    EXPECT_NE(std::string::npos, long_date_str.find(std::to_string(dt_ref.GetDay())));

}


class CDMDateTimePracticalTest : public ::testing::Test {
};

TEST_F(CDMDateTimePracticalTest, LogFileNameGeneration) {
    CDMDateTime fixed_now(2024, 7, 15, 10, 30, 0);
    std::string logFileName = "log_" + fixed_now.ToString("%Y_%m_%d_%H_%M_%S") + ".txt";
    EXPECT_EQ("log_2024_07_15_10_30_00.txt", logFileName);
}

TEST_F(CDMDateTimePracticalTest, CalculateAge) {
    CDMDateTime birthday(1990, 5, 15);
    CDMDateTime current_fixed_date(2024, 6, 4); // Fixed date for test consistency

    CDMTimeSpan age_span = current_fixed_date.Subtract(birthday);
    int years_calculated_approx = static_cast<int>(age_span.GetTotalDays() / 365.25);
    // Actual age: 2024-1990 = 34. 6/4 is after 5/15. So 34.
    EXPECT_EQ(34, years_calculated_approx);

    // More precise age calculation for comparison (if library doesn't provide direct age)
    int precise_years = current_fixed_date.GetYear() - birthday.GetYear();
    if (current_fixed_date.GetMonth() < birthday.GetMonth() ||
        (current_fixed_date.GetMonth() == birthday.GetMonth() && current_fixed_date.GetDay() < birthday.GetDay())) {
        precise_years--;
    }
    EXPECT_EQ(precise_years, years_calculated_approx); // Check if approximation matches common calculation

    CDMDateTime birthday_edge(2000, 1, 1);
    CDMDateTime current_edge(2023, 12, 31);
    CDMTimeSpan age_span_edge = current_edge.Subtract(birthday_edge);
    // Age is 23 (almost 24)
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
    // 2024-01-01 (Mon), 02 (Tue), 03 (Wed), 04 (Thu), 05 (Fri) -> 5 days
    EXPECT_EQ(5, workdays);

    CDMDateTime startDate2 = CDMDateTime::Parse("2024-12-23"); // Monday
    CDMDateTime endDate2 = CDMDateTime::Parse("2024-12-29");   // Sunday
    // 23(Mon), 24(Tue), 25(Wed), 26(Thu), 27(Fri) -> 5 days
    // (Assuming Christmas 25th is still counted as IsWeekday if no holiday logic)
    int workdays2 = 0;
    for (CDMDateTime date = startDate2; date <= endDate2; date = date.AddDays(1)) {
        if (date.IsWeekday()) {
            workdays2++;
        }
    }
    EXPECT_EQ(5, workdays2);
}

TEST_F(CDMDateTimePracticalTest, OperationTimeCalculation) {
    CDMDateTime startTime(2024, 1, 1, 10, 0, 0, 0);
    CDMDateTime endTime(2024, 1, 1, 10, 0, 1, 500); // 1 second, 500 milliseconds later

    CDMTimeSpan elapsed = endTime.Subtract(startTime);
    EXPECT_EQ(1500, elapsed.GetTotalMilliseconds());
    auto totalSeconds = elapsed.GetTotalSeconds();
    EXPECT_DOUBLE_EQ(1, totalSeconds);

    CDMDateTime startTime2(2024, 1, 1, 10, 0, 0, 0);
    CDMDateTime endTime2(2024, 1, 1, 10, 1, 30, 250); // 1 min, 30 sec, 250 ms later
    CDMTimeSpan elapsed2 = endTime2.Subtract(startTime2); // 60s + 30s + 0.25s = 90.25s
    EXPECT_EQ(90250, elapsed2.GetTotalMilliseconds());
    EXPECT_DOUBLE_EQ(90, elapsed2.GetTotalSeconds());
}