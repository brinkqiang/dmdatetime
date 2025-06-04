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
        // dt_ts_ref is for timestamp 1703512245L (2023-12-25 13:50:45 UTC)
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
    EXPECT_EQ(test_timestamp_sec, dt5_from_ts.GetTimestamp());
}

TEST_F(CDMDateTimeUsageTest, GetComponentValues) {
    EXPECT_EQ(2024, dt_ref.GetYear());
    EXPECT_EQ(12, dt_ref.GetMonth());
    EXPECT_EQ(25, dt_ref.GetDay());
    EXPECT_EQ(15, dt_ref.GetHour());
    EXPECT_EQ(30, dt_ref.GetMinute());
    EXPECT_EQ(45, dt_ref.GetSecond());
    EXPECT_EQ(3, dt_ref.GetDayOfWeek()); // 2024-12-25 is Wednesday (0=Sun, 1=Mon, ..., 3=Wed)
    EXPECT_EQ(360, dt_ref.GetDayOfYear()); // 2024 is a leap year. 31(Jan)+29(Feb)+31(Mar)+30(Apr)+31(May)+30(Jun)+31(Jul)+31(Aug)+30(Sep)+31(Oct)+30(Nov)+25(Dec)
}

TEST_F(CDMDateTimeUsageTest, TimeArithmetic) {
    CDMDateTime tomorrow = dt_ref.AddDays(1);
    EXPECT_EQ(2024, tomorrow.GetYear());
    EXPECT_EQ(12, tomorrow.GetMonth());
    EXPECT_EQ(26, tomorrow.GetDay());
    EXPECT_EQ(dt_ref.GetHour(), tomorrow.GetHour());

    CDMDateTime nextWeek = dt_ref.AddDays(7);
    EXPECT_EQ(2025, nextWeek.GetYear()); // 25 + 7 = 32, so Jan 1st of next year
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

    CDMDateTime endOfDay = dt_ref.GetEndOfDay();
    EXPECT_EQ(2024, endOfDay.GetYear());
    EXPECT_EQ(12, endOfDay.GetMonth());
    EXPECT_EQ(25, endOfDay.GetDay());
    EXPECT_EQ(23, endOfDay.GetHour());
    EXPECT_EQ(59, endOfDay.GetMinute());
    EXPECT_EQ(59, endOfDay.GetSecond());

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
    EXPECT_EQ(dt_ref.GetTimestamp(), from_ts_sec.GetTimestamp());
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

    CDMDateTime now = CDMDateTime::Now();
    EXPECT_EQ(now.GetYear(), today.GetYear());
    EXPECT_EQ(now.GetMonth(), today.GetMonth());
    EXPECT_EQ(now.GetDay(), today.GetDay());

    CDMDateTime minValue = CDMDateTime::MinValue();
    CDMDateTime maxValue = CDMDateTime::MaxValue();

    fmt::print("{} size_t={}\n", minValue.ToString(), sizeof(time_t));
    fmt::print("{} size_t={}\n", maxValue.ToString(), sizeof(time_t));
    EXPECT_TRUE(minValue < dt_ref);
    EXPECT_TRUE(maxValue > dt_ref);
    EXPECT_TRUE(minValue < maxValue);

    // Check against the values defined in CDMDateTime.h
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
    EXPECT_EQ(std::string::npos, cn_short_date_str.find("时")); // Should not contain time component
}

// 新增的测试用例，用于测试 SetDate 和 SetTime 方法
TEST_F(CDMDateTimeUsageTest, SetDateAndTimeMethods) {
    CDMDateTime dt_mutable(2024, 12, 25, 15, 30, 45); // 创建一个可变副本

    // 保存原始时间，用于后续验证
    int original_hour = dt_mutable.GetHour();
    int original_minute = dt_mutable.GetMinute();
    int original_second = dt_mutable.GetSecond();

    // 测试 SetDate 方法
    dt_mutable.SetDate(2025, 1, 10);
    EXPECT_EQ(2025, dt_mutable.GetYear());
    EXPECT_EQ(1, dt_mutable.GetMonth());
    EXPECT_EQ(10, dt_mutable.GetDay());
    EXPECT_EQ(original_hour, dt_mutable.GetHour());   // 验证时间部分未改变
    EXPECT_EQ(original_minute, dt_mutable.GetMinute()); // 验证时间部分未改变
    EXPECT_EQ(original_second, dt_mutable.GetSecond()); // 验证时间部分未改变

    // 为 SetTime 测试重置对象状态或使用新对象，以保持测试独立性
    dt_mutable = CDMDateTime(2024, 12, 25, 15, 30, 45);

    // 保存原始日期，用于后续验证
    int original_year = dt_mutable.GetYear();
    int original_month = dt_mutable.GetMonth();
    int original_day = dt_mutable.GetDay();

    // 测试 SetTime 方法
    dt_mutable.SetTime(5, 15, 55);
    EXPECT_EQ(original_year, dt_mutable.GetYear());   // 验证日期部分未改变
    EXPECT_EQ(original_month, dt_mutable.GetMonth()); // 验证日期部分未改变
    EXPECT_EQ(original_day, dt_mutable.GetDay());     // 验证日期部分未改变
    EXPECT_EQ(5, dt_mutable.GetHour());
    EXPECT_EQ(15, dt_mutable.GetMinute());
    EXPECT_EQ(55, dt_mutable.GetSecond());
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
    int years_calculated_approx = static_cast<int>(age_span.GetTotalDays() / 365.25); // Approximate age
    EXPECT_EQ(34, years_calculated_approx);

    // More precise age calculation
    int precise_years = current_fixed_date.GetYear() - birthday.GetYear();
    if (current_fixed_date.GetMonth() < birthday.GetMonth() ||
        (current_fixed_date.GetMonth() == birthday.GetMonth() && current_fixed_date.GetDay() < birthday.GetDay())) {
        precise_years--;
    }
    EXPECT_EQ(precise_years, years_calculated_approx); // Check if approximation matches precise for this case

    CDMDateTime birthday_edge(2000, 1, 1);
    CDMDateTime current_edge(2023, 12, 31);
    CDMTimeSpan age_span_edge = current_edge.Subtract(birthday_edge);
    int precise_years_edge = current_edge.GetYear() - birthday_edge.GetYear();
    if (current_edge.GetMonth() < birthday_edge.GetMonth() ||
        (current_edge.GetMonth() == birthday_edge.GetMonth() && current_edge.GetDay() < birthday_edge.GetDay())) {
        precise_years_edge--;
    }
    EXPECT_EQ(precise_years_edge, static_cast<int>(age_span_edge.GetTotalDays() / 365.25)); // For this case, 23 years
    EXPECT_EQ(23, precise_years_edge);
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
    EXPECT_EQ(5, workdays); // Mon, Tue, Wed, Thu, Fri

    CDMDateTime startDate2 = CDMDateTime::Parse("2024-12-23"); // Monday
    CDMDateTime endDate2 = CDMDateTime::Parse("2024-12-29");   // Sunday
    int workdays2 = 0;
    for (CDMDateTime date = startDate2; date <= endDate2; date = date.AddDays(1)) {
        if (date.IsWeekday()) {
            workdays2++;
        }
    }
    EXPECT_EQ(5, workdays2); // Mon, Tue, Wed, Thu, Fri
}

TEST_F(CDMDateTimePracticalTest, OperationTimeCalculation) {
    CDMDateTime startTime(2024, 1, 1, 10, 0, 0);
    CDMDateTime endTime(2024, 1, 1, 10, 0, 1);

    CDMTimeSpan elapsed = endTime.Subtract(startTime);
    auto totalSeconds = elapsed.GetTotalSeconds();
    EXPECT_DOUBLE_EQ(1, totalSeconds);

    CDMDateTime startTime2(2024, 1, 1, 10, 0, 0);
    CDMDateTime endTime2(2024, 1, 1, 10, 1, 30); // 1 minute 30 seconds
    CDMTimeSpan elapsed2 = endTime2.Subtract(startTime2);
    EXPECT_DOUBLE_EQ(90, elapsed2.GetTotalSeconds()); // 60 + 30 = 90 seconds
}

TEST_F(CDMDateTimePracticalTest, Subtract) {
    CDMDateTime startTime(2024, 1, 1, 10, 0, 0);
    CDMDateTime endTime(2024, 1, 1, 11, 30, 1);

    EXPECT_EQ(5401, endTime.Subtract(startTime).GetTotalSeconds());
    EXPECT_EQ(90, endTime.Subtract(startTime).GetTotalMinutes());
    EXPECT_EQ(1, endTime.Subtract(startTime).GetTotalHours());
}

TEST_F(CDMDateTimePracticalTest, 3000)
{
    CDMDateTime maxValue = CDMDateTime::MaxValue();

    fmt::print("{}\n", maxValue.GetYear());

    fmt::print("{}\n", maxValue.ToString());


    std::tm t{};
    t.tm_year = 3000 - 1900;    // tm_year 是自1900年起的年数
    t.tm_mon = 1 - 1;       // tm_mon 是从0开始的月份 (0=一月, 11=十二月)
    t.tm_mday = 0;            // tm_mday 是一月中的日期 (1-31)
    t.tm_hour = 0;           // tm_hour 是一天中的小时 (0-23)
    t.tm_min = 0;          // tm_min 是一小时中的分钟 (0-59)
    t.tm_sec = 0;          // tm_sec 是一分钟中的秒数 (0-59)
    t.tm_isdst = -1;            // 夏令时信息，-1表示让系统自动判断

    std::time_t tt = std::mktime(&t); // 将 std::tm 结构转换为 time_t 类型

    auto time_point_ = std::chrono::system_clock::from_time_t(tt);

    std::time_t tt = std::chrono::system_clock::to_time_t(time_point_);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &tt);
#else
    localtime_r(&tt, &local_tm);
#endif

    fmt::print("{}\n", local_tm.tm_year);

    int start_year = 2150;
    int end_year_search_limit = 3000;

    for (int year_to_test = start_year; year_to_test <= end_year_search_limit; ++year_to_test) {
        CDMDateTime dt_test_object;
        bool construction_ok = true;
        std::string error_message;

        try {
            dt_test_object = CDMDateTime(year_to_test, 1, 1, 12, 0, 0);

            int retrieved_year = dt_test_object.GetYear();
            int retrieved_month = dt_test_object.GetMonth();
            int retrieved_day = dt_test_object.GetDay();

            if (retrieved_year != year_to_test) {
                construction_ok = false;
                error_message = "Component mismatch. Expected: " + std::to_string(year_to_test) + "-01-01, "
                    + "Got: " + std::to_string(retrieved_year) + "-"
                    + std::to_string(retrieved_month) + "-"
                    + std::to_string(retrieved_day);
            }
        }
        catch (const std::runtime_error& e) {
            construction_ok = false;
            error_message = "std::runtime_error caught: " + std::string(e.what());
        }
        catch (const std::exception& e) {
            construction_ok = false;
            error_message = "std::exception caught: " + std::string(e.what());
        }
        catch (...) {
            construction_ok = false;
            error_message = "Unknown exception caught during CDMDateTime construction or validation.";
        }

        if (!construction_ok) {
            std::cerr << "Error encountered when testing year: " << year_to_test << std::endl;
            std::cerr << "Error details: " << error_message << std::endl;
            break;
        }
    }
}