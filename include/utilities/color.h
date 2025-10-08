#pragma once

#include <string>
#include <string_view>

namespace color {

  /*
   * Shell color codes to use with ostreams,
   * example: std::cout << "normal, " << color::BOLD << "bold, " << color::RESET << "normal again" << std::endl;
   */

  inline constexpr std::string_view BOLD = "\033[1m";
  inline constexpr std::string_view DISABLE = "\033[02m";
  inline constexpr std::string_view ITALIC = "\033[03m";
  inline constexpr std::string_view UNDERLINE = "\033[4m";
  inline constexpr std::string_view BLINK = "\033[05m";
  inline constexpr std::string_view HIGHLIGHT = "\033[07m", INVERT = HIGHLIGHT;
  inline constexpr std::string_view INVISIBLE = "\033[08m";
  inline constexpr std::string_view STRIKETHROUGH = "\033[09m";
  inline constexpr std::string_view DOUBLE_UNDERLINE = "\033[21m";
  inline constexpr std::string_view OVERLINE = "\033[53m";
  inline constexpr std::string_view PALETTE_0 = "\033[30m", DARK_GREY = PALETTE_0;
  inline constexpr std::string_view PALETTE_1 = "\033[31m", DARK_RED = PALETTE_1;
  inline constexpr std::string_view PALETTE_2 = "\033[32m", DARK_GREEN = PALETTE_2;
  inline constexpr std::string_view PALETTE_3 = "\033[33m", DARK_YELLOW = PALETTE_3;
  inline constexpr std::string_view PALETTE_4 = "\033[34m", DARK_BLUE = PALETTE_4;
  inline constexpr std::string_view PALETTE_5 = "\033[35m", DARK_PINK = PALETTE_5;
  inline constexpr std::string_view PALETTE_6 = "\033[36m", DARK_AQUA = PALETTE_6;
  inline constexpr std::string_view PALETTE_7 = "\033[37m", DARK_WHITE = PALETTE_7;
  inline constexpr std::string_view PALETTE_8 = "\033[90m", GREY = PALETTE_8;
  inline constexpr std::string_view PALETTE_9 = "\033[91m", RED = PALETTE_9;
  inline constexpr std::string_view PALETTE_10 = "\033[92m", GREEN = PALETTE_10;
  inline constexpr std::string_view PALETTE_11 = "\033[93m", YELLOW = PALETTE_11;
  inline constexpr std::string_view PALETTE_12 = "\033[94m", BLUE = PALETTE_12;
  inline constexpr std::string_view PALETTE_13 = "\033[95m", PINK = PALETTE_13;
  inline constexpr std::string_view PALETTE_14 = "\033[96m", AQUA = PALETTE_14;
  inline constexpr std::string_view PALETTE_15 = "\033[97m", WHITE = PALETTE_15;
  inline constexpr std::string_view BG_PALETTE_0 = "\033[40m", BG_DARK_GREY = BG_PALETTE_0;
  inline constexpr std::string_view BG_PALETTE_1 = "\033[41m", BG_DARK_RED = BG_PALETTE_1;
  inline constexpr std::string_view BG_PALETTE_2 = "\033[42m", BG_DARK_GREEN = BG_PALETTE_2;
  inline constexpr std::string_view BG_PALETTE_3 = "\033[43m", BG_DARK_YELLOW = BG_PALETTE_3;
  inline constexpr std::string_view BG_PALETTE_4 = "\033[44m", BG_DARK_BLUE = BG_PALETTE_4;
  inline constexpr std::string_view BG_PALETTE_5 = "\033[45m", BG_DARK_PINK = BG_PALETTE_5;
  inline constexpr std::string_view BG_PALETTE_6 = "\033[46m", BG_DARK_AQUA = BG_PALETTE_6;
  inline constexpr std::string_view BG_PALETTE_7 = "\033[47m", BG_DARK_WHITE = BG_PALETTE_7;
  inline constexpr std::string_view BG_PALETTE_8 = "\033[100m", BG_GREY = BG_PALETTE_8;
  inline constexpr std::string_view BG_PALETTE_9 = "\033[101m", BG_RED = BG_PALETTE_9;
  inline constexpr std::string_view BG_PALETTE_10 = "\033[102m", BG_GREEN = BG_PALETTE_10;
  inline constexpr std::string_view BG_PALETTE_11 = "\033[103m", BG_YELLOW = BG_PALETTE_11;
  inline constexpr std::string_view BG_PALETTE_12 = "\033[104m", BG_BLUE = BG_PALETTE_12;
  inline constexpr std::string_view BG_PALETTE_13 = "\033[105m", BG_PINK = BG_PALETTE_13;
  inline constexpr std::string_view BG_PALETTE_14 = "\033[106m", BG_AQUA = BG_PALETTE_14;
  inline constexpr std::string_view BG_PALETTE_15 = "\033[107m", BG_WHITE = BG_PALETTE_15;
  inline constexpr std::string_view BG_BLACK = "\033[48m"; // default background color
  inline constexpr std::string_view ENDC = "\033[0m", RESET = ENDC;  // reset all colors to default

  /**
   * @brief returns a formatted version of 'message'
   *        Example: color::format("this is blue + underlined", {color::BLUE, color::UNDERLINE}, true)
   *
   * @param message the string that should be formatted
   * @param formats a {...} list of the color::formats that should be used
   * @param append_reset whether to append a color::RESET to the end so subsequent text looks normal
   * @return std::string i.e. the formatted string
   */
  inline std::string format(const std::string& message,
                     std::initializer_list<std::string_view> formats = {color::RED},
                     bool append_reset = true) {
    std::string result;
    for (const auto& format : formats) {
        result += format;
    }
    result += message;
    if (append_reset) {
        result += color::RESET;
    }
    return result;
  }

} // namespace color
