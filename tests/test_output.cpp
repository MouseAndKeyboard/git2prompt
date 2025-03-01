#include <gtest/gtest.h>
#include "output_formatter.hpp"

TEST(OutputTest, FormatTextBasic) {
    std::vector<FileInfo> files{
        {"src/main.cpp", "int main() { return 0; }"},
        {"README.md", "# This is readme\nsome content"}
    };
    auto result = format_text(files, "", false, true);
    EXPECT_TRUE(result.ok);
    EXPECT_FALSE(result.data.empty());
    EXPECT_GT(result.tokens, 0);
    EXPECT_NE(std::string::npos, result.data.find("----"));
    EXPECT_NE(std::string::npos, result.data.find("--END--"));
}

TEST(OutputTest, FormatJsonBasic) {
    std::vector<FileInfo> files{
        {"src/main.cpp", "int main() { return 0; }"},
        {"README.md", "# This is readme\nsome content"}
    };
    auto result = format_json(files, "", false, true);
    EXPECT_TRUE(result.ok);
    EXPECT_FALSE(result.data.empty());
    EXPECT_NE(std::string::npos, result.data.find("\"files\""));
    EXPECT_NE(std::string::npos, result.data.find("\"preamble\""));
    EXPECT_NE(std::string::npos, result.data.find("\"token_estimate\""));
    EXPECT_GT(result.tokens, 0);
}
