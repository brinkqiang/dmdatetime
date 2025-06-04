#include "./dmdatetime.h"
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

class CDMDateTimePracticalTest : public ::testing::Test {
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

TEST_F(CDMDateTimePracticalTest, 3000)
{
    CDMDateTime maxValue = CDMDateTime::MaxValue();

    fmt::print("{}\n", maxValue.GetYear());

    fmt::print("{}\n", maxValue.ToString());

    CDMDateTime xdata;
    xdata.SetDateTime(3000, 1, 1, 12, 0, 0);

    fmt::print("{} {}\n", xdata.ToString(), xdata.GetYear());

    int start_year = 2150;
    int end_year_search_limit = 3000;

    for (int year_to_test = start_year; year_to_test <= end_year_search_limit; ++year_to_test) {
        CDMDateTime dt_test_object;
        std::string error_message;

        try {
            dt_test_object = CDMDateTime(year_to_test, 1, 1, 12, 0, 0);

            int retrieved_year = dt_test_object.GetYear();
            int retrieved_month = dt_test_object.GetMonth();
            int retrieved_day = dt_test_object.GetDay();

            if (retrieved_year != year_to_test) {
                std::cerr << "Error encountered when testing year: " << year_to_test << std::endl;
                std::cerr << "Error details: " << error_message << std::endl;
                break;
            }
        }
        catch (...) {
            fmt::print("{}\n", "Unknown exception caught during CDMDateTime construction or validation.");
        }
    }
}