#ifndef BOUNDLESS_LOG_FILE
#define BOUNDLESS_LOG_FILE
#include <chrono>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <source_location>
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

inline void print_source_loc(std::ostream& stm,
                             const std::source_location& loc) {
    stm << '[' << loc.file_name() << "::" << loc.function_name()
        << "::" << loc.line() << ']';
}
inline void print_time(std::ostream& stm) {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<uint64_t, std::chrono::milliseconds> now_ms =
        std::chrono::duration_cast<uint64_t, std::chrono::milliseconds>(
            now.time_since_epoch());
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    stm << '[';
    stm << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S.")
        << (now_ms.count() % 1000) << ']';
}

template <typename... Types>
void print_error_internal(const std::source_location loc,
                          const char* type,
                          const Types&... args) {
    std::cerr << ConsoleColor::Red;
    print_time(std::cerr);
    print_source_loc(std::cerr, loc);
    std::cerr << '[' << type << ']' << ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cerr << args << ' '; }(), 0)...};
    std::cerr << '\n';
}
template <typename... Types>
void print_warning_internal(const std::source_location loc,
                            const char* type,
                            const Types&... args) {
    std::cerr << ConsoleColor::Yellow;
    print_time(std::cerr);
    print_source_loc(std::cerr,loc);
    std::cerr << '[' << type << ']' << ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cerr << args << ' '; }(), 0)...};
    std::cerr << '\n';
}
template <typename... Types>
void print_log_internal(const char* type, const Types&... args) {
    std::cout << ConsoleColor::Green << '[' << type << ']'
              << ConsoleColor::None;
    std::initializer_list<int>{([&args] { std::cout << args << ' '; }(), 0)...};
    std::cout << '\n';
}
#define print_error(type, ...) \
    print_error_internal(std::source_location::current(), type, __VA_ARGS__)
#define print_warning(type, ...) \
    print_error_internal(std::source_location::current(), type, __VA_ARGS__)
#define print_log(type, ...) print_log_internal(type, __VA_ARGS__)
}  // namespace BL
#endif  //! BOUNDLESS_LOG_FILE