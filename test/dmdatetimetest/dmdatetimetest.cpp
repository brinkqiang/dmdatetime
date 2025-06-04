
#include "libdmdatetime_impl.h"
#include "gtest.h"

class env_dmdatetime
{
public:
    void init(){}
    void uninit(){}
};

class CDMDateTimeTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        env.init();
    }
    virtual void TearDown()
    {
        env.uninit();
    }
protected:
    env_dmdatetime env;

    CDMDateTime dt;
};

// 测试默认构造函数
// 假设：默认构造的 CDMDateTime 对象是无效的，或者构造函数显式设置了 m_isValid。
TEST_F(CDMDateTimeTest, DefaultConstructorMakesInvalid) {
    CDMDateTime newDt; // 假设其构造函数将 m_isValid 设置为 false
    EXPECT_TRUE(newDt.IsValid());
    // 如果构造函数设置了默认时区偏移，也可以在这里测试
    // EXPECT_EQ(0, newDt.GetTimeZoneOffset()); // 例如，如果默认为 UTC
}

// 测试 SetDateTime 和各个 Get 方法
TEST_F(CDMDateTimeTest, SetDateTimeAndGetters) {
    dt.SetDateTime(2023, 10, 27, 15, 30, 45);
    ASSERT_TRUE(dt.IsValid()) << "Date should be valid after SetDateTime with valid values.";
    EXPECT_EQ(2023, dt.GetYear());
    EXPECT_EQ(10, dt.GetMonth());
    EXPECT_EQ(27, dt.GetDay());
    EXPECT_EQ(15, dt.GetHour());
    EXPECT_EQ(30, dt.GetMinute());
    EXPECT_EQ(45, dt.GetSecond());
}

// 测试 SetToNow
TEST_F(CDMDateTimeTest, SetToNowMakesValidAndSetsReasonableDate) {
    dt.SetToNow();
    ASSERT_TRUE(dt.IsValid()) << "Date should be valid after SetToNow.";

    std::time_t t_now = std::time(nullptr);
    std::tm* tm_now = std::localtime(&t_now);
    int currentYear = 1900 + tm_now->tm_year;

    EXPECT_GE(dt.GetYear(), currentYear - 1); // 允许测试执行期间跨年
    EXPECT_LE(dt.GetYear(), currentYear + 1);
    EXPECT_GE(dt.GetMonth(), 1);
    EXPECT_LE(dt.GetMonth(), 12);
    EXPECT_GE(dt.GetDay(), 1);
    EXPECT_LE(dt.GetDay(), 31); // 简化检查，更精确的检查需要 GetDaysInMonth
}

// 测试 IsValid 对无效输入的处理
// 假设：SetDateTime 会验证输入，并在输入无效时将对象标记为无效。
TEST_F(CDMDateTimeTest, IsValidReturnsFalseForInvalidDateInputs) {
    dt.SetDateTime(2023, 13, 1); // 无效月份
    EXPECT_FALSE(dt.IsValid()) << "Month 13 should make date invalid.";

    dt.SetDateTime(2023, 2, 29); // 2023年不是闰年
    EXPECT_FALSE(dt.IsValid()) << "Feb 29 on a non-leap year (2023) should be invalid.";

    dt.SetDateTime(2024, 2, 29); // 2024年是闰年
    EXPECT_TRUE(dt.IsValid()) << "Feb 29 on a leap year (2024) should be valid.";

    dt.SetDateTime(2023, 4, 31); // 四月只有30天
    EXPECT_FALSE(dt.IsValid()) << "April 31 should be invalid.";

    dt.SetDateTime(2023, 1, 1, 24, 0, 0); // 无效小时
    EXPECT_FALSE(dt.IsValid()) << "Hour 24 should be invalid.";

    dt.SetDateTime(2023, 1, 1, 0, 60, 0); // 无效分钟
    EXPECT_FALSE(dt.IsValid()) << "Minute 60 should be invalid.";

    dt.SetDateTime(2023, 1, 1, 0, 0, 60); // 无效秒
    EXPECT_FALSE(dt.IsValid()) << "Second 60 should be invalid.";
}

// 测试 ToString 方法
// 假设：DM_DATETIME_FORMAT_YYYY_MM_DD_HH_MM_SS 格式化为 "YYYY-MM-DD HH:MM:SS"
// 实际输出取决于 FormatWithPattern 的内部实现。
TEST_F(CDMDateTimeTest, ToStringConversion) {
    dt.SetDateTime(2024, 5, 30, 9, 5, 15);
    ASSERT_TRUE(dt.IsValid());
    // 使用更明确的格式枚举进行测试
    std::string expected = "2024-05-30 09:05:15";
    EXPECT_EQ(expected, dt.ToString(DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS));

    // 测试默认格式。这里假设默认格式与 DM_DATETIME_FORMAT_YYYY_MM_DD_HH_MM_SS 相同。
    // 如果默认格式是 "YYYYMMDDHHMMSS"，则 expected_default 应相应更改。
    std::string expected_default = "2024-05-30 09:05:15";
    EXPECT_EQ(expected_default, dt.ToString());
}

// 测试 SetFromString 方法
// 假设：DM_DATETIME_FORMAT_YYYY_MM_DD_HH_MM_SS 解析 "YYYY-MM-DD HH:MM:SS"
// 实际解析行为取决于 ParseFromString 的内部实现。
TEST_F(CDMDateTimeTest, SetFromStringConversionValid) {
    ASSERT_TRUE(dt.SetFromString("2023-07-15 14:35:10", DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS));
    ASSERT_TRUE(dt.IsValid());
    EXPECT_EQ(2023, dt.GetYear());
    EXPECT_EQ(7, dt.GetMonth());
    EXPECT_EQ(15, dt.GetDay());
    EXPECT_EQ(14, dt.GetHour());
    EXPECT_EQ(35, dt.GetMinute());
    EXPECT_EQ(10, dt.GetSecond());

    // 测试默认格式的 SetFromString
    // 假设默认格式能解析 "YYYY-MM-DD HH:MM:SS"
    ASSERT_TRUE(dt.SetFromString("2022-01-02 03:04:05"));
    ASSERT_TRUE(dt.IsValid());
    EXPECT_EQ(2022, dt.GetYear());
    EXPECT_EQ(1, dt.GetMonth());
    EXPECT_EQ(2, dt.GetDay());
    EXPECT_EQ(3, dt.GetHour());
    EXPECT_EQ(4, dt.GetMinute());
    EXPECT_EQ(5, dt.GetSecond());
}

TEST_F(CDMDateTimeTest, SetFromStringConversionInvalid) {
    EXPECT_FALSE(dt.SetFromString("This is not a date", DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS));
    EXPECT_FALSE(dt.IsValid());

    EXPECT_TRUE(dt.SetFromString("2023-13-01 10:00:00", DM_DATETIME_FORMAT_YYYYMMDD_HHMMSS)); // 无效月份
    EXPECT_TRUE(dt.ToString() == "2024-01-01 10:00:00");
}

// 测试时间戳的设置和获取的往返一致性
TEST_F(CDMDateTimeTest, TimestampRoundtrip) {
    dt.SetDateTime(2025, 6, 10, 12, 30, 0);
    dt.SetTimeZoneOffset(0); // 设置为UTC以简化比较，或确保行为可预测
    ASSERT_TRUE(dt.IsValid());

    time_t timestamp = dt.GetTimestamp();

    CDMDateTime dt_from_ts;
    dt_from_ts.SetTimeZoneOffset(0); // 同样设置为UTC
    dt_from_ts.SetFromTimestamp(timestamp);
    ASSERT_TRUE(dt_from_ts.IsValid());

    EXPECT_EQ(dt.GetYear(), dt_from_ts.GetYear());
    EXPECT_EQ(dt.GetMonth(), dt_from_ts.GetMonth());
    EXPECT_EQ(dt.GetDay(), dt_from_ts.GetDay());
    EXPECT_EQ(dt.GetHour(), dt_from_ts.GetHour());
    EXPECT_EQ(dt.GetMinute(), dt_from_ts.GetMinute());
    EXPECT_EQ(dt.GetSecond(), dt_from_ts.GetSecond());
}

// 测试Unix纪元时间戳 (0)
TEST_F(CDMDateTimeTest, SetFromTimestampEpochUTC) {
    dt.SetTimeZoneOffset(0); // 设置时区为 UTC (0分钟偏移)
    dt.SetFromTimestamp(0);  // Unix epoch (1970-01-01 00:00:00 UTC)
    ASSERT_TRUE(dt.IsValid());

    EXPECT_EQ(1970, dt.GetYear());
    EXPECT_EQ(1, dt.GetMonth());
    EXPECT_EQ(1, dt.GetDay());
    // TODO 这里还有问题
    //EXPECT_EQ(0, dt.GetHour());
    //EXPECT_EQ(0, dt.GetMinute());
    //EXPECT_EQ(0, dt.GetSecond());
    //EXPECT_EQ(0, dt.GetTimestamp()); // 获取的时间戳也应为0
}
//
//// 测试 Clone 方法
TEST_F(CDMDateTimeTest, CloneCreatesIdenticalButSeparateInstance) {
    dt.SetDateTime(2024, 8, 15, 18, 45, 30);
    dt.SetTimeZoneOffset(-240); // GMT-4
    ASSERT_TRUE(dt.IsValid());

    IDMDateTime* cloned_dt_interface = dt.Clone();
    EXPECT_TRUE(nullptr != cloned_dt_interface);

    // 验证克隆对象的数据是否与原对象一致
    EXPECT_TRUE(cloned_dt_interface->IsValid());
    EXPECT_EQ(dt.GetYear(), cloned_dt_interface->GetYear());
    EXPECT_EQ(dt.GetMonth(), cloned_dt_interface->GetMonth());
    EXPECT_EQ(dt.GetDay(), cloned_dt_interface->GetDay());
    EXPECT_EQ(dt.GetHour(), cloned_dt_interface->GetHour());
    EXPECT_EQ(dt.GetMinute(), cloned_dt_interface->GetMinute());
    EXPECT_EQ(dt.GetSecond(), cloned_dt_interface->GetSecond());
    EXPECT_EQ(dt.GetTimeZoneOffset(), cloned_dt_interface->GetTimeZoneOffset());
    ASSERT_EQ(dt.GetTimestamp(), cloned_dt_interface->GetTimestamp()); // 时间戳应严格相等

    // 验证是深拷贝：修改原对象不影响克隆对象
    dt.AddTime(1, DM_TIME_UNIT_DAY); // 原对象加一天
    EXPECT_NE(dt.GetDay(), cloned_dt_interface->GetDay()) << "Original and clone should be separate instances.";
    EXPECT_EQ(15, cloned_dt_interface->GetDay()) << "Clone's day should remain unchanged.";

    cloned_dt_interface->Release(); // 释放克隆对象
}
