#include <gtest/gtest.h>

TEST(ExampleTest, HandlesZeroInput) {
    EXPECT_EQ(0, 0);
}

TEST(ExampleTest, HandlesPositiveInput) {
    EXPECT_EQ(1 + 1, 2);
}