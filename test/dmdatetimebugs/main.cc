#include "dmdatetime_bugs.h"
#include <string>
#include <vector>
#include <numeric>
#include "gtest.h"
#include "dmformat.h"
#include "dmfix_win_utf8.h"

class CDMDateTimePracticalTest : public ::testing::Test {
};

TEST_F(CDMDateTimePracticalTest, 2000)
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