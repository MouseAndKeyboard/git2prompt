#include <gtest/gtest.h>
#include "token_count.hpp"

TEST(TokenCountTest, BasicCount) {
    std::string text = "This is a test\nwith multiple lines\n";
    auto count = approximate_token_count(text);
    // total words: 7
    EXPECT_EQ(count, 7);
}

TEST(TokenCountTest, EmptyString) {
    std::string text = "";
    auto count = approximate_token_count(text);
    EXPECT_EQ(count, 0);
}
