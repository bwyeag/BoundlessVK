#ifndef BOUNDLESS_LOG_FILE
#define BOUNDLESS_LOG_FILE
#include <initializer_list>
#include <iostream>
#include <string>
#define IS_WINDOWS                                             \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
        defined(__NT__) && !defined(__CYGWIN__)

#if IS_WINDOWS
#include <Windows.h>
#endif  // IS_WINDOWS
namespace BL {
enum class ConsoleColor {
    Green,
    Red,
    Blue,
    White,
    Black,
    Yellow,
    Purple,
    Gray,
    Cyan,
    None,
    GreenIntensity,
    RedIntensity,
    BlueIntensity,
    WhiteIntensity,
    BlackIntensity,
    YellowIntensity,
    PurpleIntensity,
    GrayIntensity,
    CyanIntensity
};

enum class ConsoleBackgroundColor {
    Green,
    Red,
    Blue,
    White,
    Black,
    Yellow,
    Purple,
    Gray,
    Cyan,
    None
};

#if IS_WINDOWS
WORD getColorCode(ConsoleColor color);
WORD getBackgroundColorCode(ConsoleBackgroundColor color);
#else
std::string getColorCode(ConsoleColor color);
std::string getBackgroundColorCode(ConsoleBackgroundColor color);
#endif

std::ostream& operator<<(std::ostream& os, ConsoleColor data);
std::ostream& operator<<(std::ostream& os, ConsoleBackgroundColor data);

template <typename... Types>
void print_error(const char* type, const Types&... args) {
#ifdef BL_DEBUG
    std::cerr << ConsoleColor::Red << '[' << type << ']' <<  ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cerr << args << ' '; }(), 0)...};
    std::cerr << '\n';
#endif  // BL_DEBUG
}
template <typename... Types>
void print_warning(const char* type, const Types&... args) {
    std::cout << ConsoleColor::Yellow << '[' << type << ']' <<  ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cout << args << ' '; }(), 0)...};
    std::cout << '\n';
}
template <typename... Types>
void print_log(const char* type, const Types&... args) {
    std::cout << ConsoleColor::Green << '[' << type << ']' <<  ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cout << args << ' '; }(), 0)...};
    std::cout << '\n';
}
}  // namespace BL
#endif  //! BOUNDLESS_LOG_FILE