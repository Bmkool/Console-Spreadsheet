#include <gtest/gtest.h>
#include <fstream>

#include "../src/Spreadsheet.hpp"

constexpr auto TEST_FILENAME = "TEST_FILE.txt";

class FileToSpreadsheetTests : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(TEST_FILENAME);
        output.open(TEST_FILENAME);
        if (!output.good()) {
            FAIL() << "Unable to create test file";
        }
    }

    void TearDown() override {
        output.close(); // Close file if not already done
        std::remove(TEST_FILENAME);
    }

    std::ofstream output;
    std::vector<std::vector<std::string>> ss;
};

TEST_F(FileToSpreadsheetTests, InputFileInvalid) {
    EXPECT_FALSE(fileToSpreadsheet("invalid.txt", ss));
}

TEST_F(FileToSpreadsheetTests, InputFileValid) {
    EXPECT_TRUE(fileToSpreadsheet(TEST_FILENAME, ss));
}


TEST_F(FileToSpreadsheetTests, SingleRowSingleColumnElementSS) {
    output << "1";
    output.close();
    std::vector<std::vector<std::string>> exp
    {
        {"1"}
    };
    EXPECT_TRUE(fileToSpreadsheet(TEST_FILENAME, ss));
    EXPECT_EQ(exp,ss);
}

TEST_F(FileToSpreadsheetTests, SingleRowMultipleColumnSS) {
    output << "1\tsd";
    output.close();
    std::vector<std::vector<std::string>> exp
    {
        {"1", "sd"}
    };
    EXPECT_TRUE(fileToSpreadsheet(TEST_FILENAME, ss));
    EXPECT_EQ(exp,ss);
}

TEST_F(FileToSpreadsheetTests, MultipleRowMultipleColumnSS) {
    output << "1\t5\n6\n\t=A1+A2";
    output.close();
    std::vector<std::vector<std::string>> exp
    {
        {"1", "5"},
        {"6"},
        {"", "=A1+A2"}
    };
    EXPECT_TRUE(fileToSpreadsheet(TEST_FILENAME, ss));
    EXPECT_EQ(exp,ss);
}

TEST(ProcessSpreadsheetTests, AllIntegersNoChange) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "-3"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst(exp);
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, InvalidInputFloat) {
    const std::vector<std::vector<std::string>> exp
    {
        {"2", "3.4"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst(exp);
    ASSERT_FALSE(processSpreadsheet(tst));
}

TEST(ProcessSpreadsheetTests, InvalidInputNonInteger) {
    const std::vector<std::vector<std::string>> exp
    {
        {"2", "d"},
        {"a"}
    };
    std::vector<std::vector<std::string>> tst(exp);
    ASSERT_FALSE(processSpreadsheet(tst));
}

TEST(ProcessSpreadsheetTests, InvalidInputIntOverflow) {
    const std::vector<std::vector<std::string>> exp
    {
        {"2", "9999999999999"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst(exp);
    ASSERT_FALSE(processSpreadsheet(tst));
}

TEST(ProcessSpreadsheetTests, FormulaNoReference) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "2"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=5-3"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}