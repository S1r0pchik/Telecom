#include "processor.h"

#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

using namespace factory;

void CreateTestInputFile(const std::string &filename, const std::string &content) {
    std::ofstream out(filename);
    out << content;
}

class ProcessorTest : public ::testing::Test {
public:
    std::string RunWithInput(const std::string &content) const {
        CreateTestInputFile("test_in.txt", content);
        RunProcessor();
        return cout_buffer.str();
    }

    static void RunProcessor() {
        Processor processor;
        char arg0[] = "test_run";
        char arg1[] = "test_in.txt";
        char *argv[] = {arg0, arg1};
        processor.Solve(2, argv);
    }

private:
    std::stringstream cout_buffer;
    std::streambuf *old_cout = nullptr;

    void SetUp() override {
        old_cout = std::cout.rdbuf(cout_buffer.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(old_cout);
        std::remove("test_in.txt");
    }
};

TEST_F(ProcessorTest, ValidInputMatchesExpectedOutput) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 6\n"
                              "3 0 1 0\n"
                              "2 1 0\n";

    const std::string expected = "start 0 0 0 0\n"
                                 "start 0 3 1 1\n"
                                 "finish 3 0 0 0\n"
                                 "wait 3 0 1 1 1\n"
                                 "start 3 1 1 0\n"
                                 "finish 6 3 1 1\n"
                                 "ready 6 3 1\n"
                                 "start 6 4 0 1\n"
                                 "finish 7 1 1 0\n"
                                 "ready 7 1 0\n"
                                 "start 7 2 0 0\n"
                                 "finish 10 2 0 0\n"
                                 "start 10 2 1 0\n"
                                 "finish 11 4 0 1\n"
                                 "wait 11 4 1 0 0\n"
                                 "start 11 0 1 1\n"
                                 "finish 14 2 1 0\n"
                                 "ready 14 2 0\n"
                                 "start 14 4 1 0\n"
                                 "finish 17 0 1 1\n"
                                 "ready 17 0 1\n"
                                 "finish 18 4 1 0\n"
                                 "ready 18 4 0\n"
                                 "stop 18\n";

    EXPECT_EQ(RunWithInput(input), expected);
}

using ProcessorDeathTest = ProcessorTest;

TEST_F(ProcessorDeathTest, InvalidLetterInMatrixExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 s\n"
                              "3 0 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);

    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, TrashAtTheEndOfLineExitsWithCode1) {
    const std::string input = "3 2 trash\n"
                              "3 5\n"
                              "4 6\n"
                              "3 0 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);

    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, ProductLevelOutOfBoundsExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 6\n"
                              "3 99 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);

    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, NotEnoughArgumentsToSolveExitsWithoutCrashing) {
    Processor processor;
    char arg0[] = "test";
    char *argv[] = {arg0};

    testing::internal::CaptureStderr();
    processor.Solve(1, argv);
    const std::string output = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(output.find("Usage:") != std::string::npos);
}

TEST_F(ProcessorDeathTest, NegativeMExitsWithCode1) {
    const std::string input = "-3 2\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, MExceeds100ExitsWithCode1) {
    const std::string input = "101 2\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, NegativeTimeMatrixValueExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "-4 6\n"
                              "3 0 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, TimeExceedsMaxExitsWithCode1) {
    const std::string input = "3 2\n"
                              "10001 5\n"
                              "4 6\n"
                              "3 0 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, NegativeQueueCountExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 6\n"
                              "-3 0 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, TotalProductsExceed100000ExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 6\n"
                              "100001 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, NegativeProductLevelExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n"
                              "4 6\n"
                              "3 -1 1 0\n"
                              "2 1 0\n";
    CreateTestInputFile("test_in.txt", input);
    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}

TEST_F(ProcessorDeathTest, UnexpectedEndOfFileExitsWithCode1) {
    const std::string input = "3 2\n"
                              "3 5\n";
    CreateTestInputFile("test_in.txt", input);

    EXPECT_EXIT(RunProcessor(), ::testing::ExitedWithCode(1), ".*");
}