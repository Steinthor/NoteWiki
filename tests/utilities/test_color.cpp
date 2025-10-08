#include <gtest/gtest.h>
#include <cmath>
#include <sstream>

#include "color.h"

TEST(ColorTest, red) {
    std::ostringstream test;
    test << color::BOLD;
    EXPECT_EQ(color::BOLD, test.str());
}