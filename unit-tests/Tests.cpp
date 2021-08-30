#include <gtest/gtest.h>
#include <fstream>

#include "../src/Spreadsheet.hpp"
#include "../src/Spreadsheet.cpp"

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
    output << "1\t5\n6\n\t\t=A1+A2";
    output.close();
    std::vector<std::vector<std::string>> exp
    {
        {"1", "5"},
        {"6"},
        {"", "", "=A1+A2"}
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

TEST(ProcessSpreadsheetTests, FormulaNoReferenceAddSub) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "8"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=5-3+1+5"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaNoReferenceAddSubMultDiv) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "-4"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=5-3*2*2+9/3"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaInvalidTokenOperatorFirst) {
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=-*3"},
        {"2"}
    };
    ASSERT_FALSE(processSpreadsheet(tst));
}

TEST(ProcessSpreadsheetTests, FormulaInvalidTokenOperatorLast) {
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=3*6*"},
        {"2"}
    };
    ASSERT_FALSE(processSpreadsheet(tst));
}

TEST(ReferenceToRowCol, InvalidNoCol) {
    auto pact = refToRowCol("1");
    auto pexp =std::pair<int,int>(-1,-1);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, InvalidNoRow) {
    auto pact = refToRowCol("A");
    auto pexp =std::pair<int,int>(-1,-1);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, InvalidSymbolCol) {
    auto pact = refToRowCol(">1");
    auto pexp =std::pair<int,int>(-1,-1);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, InvalidSymbolRow) {
    auto pact = refToRowCol("G,");
    auto pexp =std::pair<int,int>(-1,-1);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, ValidRowColA1) {
    auto pact = refToRowCol("A1");
    auto pexp =std::pair<int,int>(0,0);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, ValidRowColZ9) {
    auto pact = refToRowCol("Z9");
    auto pexp =std::pair<int,int>(8,25);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, ValidRowColBA11) {
    auto pact = refToRowCol("BA11");
    auto pexp =std::pair<int,int>(10,52);
    EXPECT_EQ(pexp, pact);
}

TEST(ReferenceToRowCol, ValidARowColAAA111) {
    auto pact = refToRowCol("AAA111");
    auto pexp =std::pair<int,int>(110,702);
    EXPECT_EQ(pexp, pact);
}

TEST(ProcessSpreadsheetTests, FormulaReferenceOtherBox) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "1"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=A1"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaReferenceSelf) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "#ERROR"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=B1"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaThreeWayCircular) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "#ERROR"},
        {"#ERROR", "#ERROR"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=A2"},
        {"=B2", "=B1"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaNAN) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "#NAN", "#NAN"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=B2", "=D1"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ProcessSpreadsheetTests, FormulaNANPropogation) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "#NAN", "#NAN"},
        {"2"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=B2", "=B1+3"},
        {"2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}


TEST(ExampleTests, Sample1) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "2"},
        {"", "3"},
        {"","","6"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "2"},
        {"", "3"},
        {"","","=A1+B1+B2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ExampleTests, Sample2) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "2"},
        {"", "3"},
        {"","","#NAN"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "2"},
        {"", "3"},
        {"","","=A1+B1+A2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ExampleTests, Sample3) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "2"},
        {"", "3"},
        {"","","0"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=A1*2"},
        {"", "=A1+B1"},
        {"","","=A1+B1-B2"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}

TEST(ExampleTests, Sample4) {
    const std::vector<std::vector<std::string>> exp
    {
        {"1", "#ERROR"},
        {"", "3"},
        {"","","#ERROR"}
    };
    std::vector<std::vector<std::string>> tst
    {
        {"1", "=B2-C3"},
        {"", "3"},
        {"","","=A1+B1"}
    };
    ASSERT_TRUE(processSpreadsheet(tst));
    EXPECT_EQ(exp,tst);
}