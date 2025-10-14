#include <gtest/gtest.h>
#include <cmath>
#include <sstream>

#include "color.h"

TEST(ColorTest, Red) {
    std::ostringstream test;
    test << color::BOLD;
    EXPECT_EQ(color::BOLD, test.str());
}

TEST(ColorTest, DefaultFormatRed) {
    std::string input = "Testing default color formatting";
    std::string formatted_input = std::string(color::RED) + input + std::string(color::RESET);
    std::string output = color::format(input);
    EXPECT_EQ(formatted_input, output);
}

TEST(ColorTest, CustomFormatNoReset) {
    std::string input = "Testing color formatting";
    std::string formatted_input = std::string(color::RED) + input;
    std::string output = color::format(input, {color::RED}, false);
    EXPECT_EQ(formatted_input, output);
}

TEST(ColorTest, CustomFormatBlue) {
    std::string input = "Testing color formatting";
    std::string formatted_input = std::string(color::BLUE) + input + std::string(color::RESET);
    std::string output = color::format(input, {color::BLUE});
    EXPECT_EQ(formatted_input, output);
}

TEST(ColorTest, CustomFormatBlueBold) {
    std::string input = "Testing color formatting";
    std::string formatted_input = std::string(color::BLUE) + std::string(color::BOLD) + input + std::string(color::RESET);
    std::string output = color::format(input, {color::BLUE, color::BOLD});
    EXPECT_EQ(formatted_input, output);
}
