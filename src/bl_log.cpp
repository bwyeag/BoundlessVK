#include "bl_log.hpp"

namespace BL {
#if IS_WINDOWS
WORD getColorCode(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::Green:
            return FOREGROUND_GREEN;
        case ConsoleColor::Black:
            return 0;
        case ConsoleColor::Blue:
            return FOREGROUND_BLUE;
        case ConsoleColor::Gray:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
        case ConsoleColor::Purple:
            return FOREGROUND_BLUE | FOREGROUND_RED;
        case ConsoleColor::Red:
            return FOREGROUND_RED;
        case ConsoleColor::White:
            return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
        case ConsoleColor::Cyan:
            return FOREGROUND_BLUE | FOREGROUND_GREEN;
        case ConsoleColor::Yellow:
            return FOREGROUND_RED | FOREGROUND_GREEN;
        case ConsoleColor::None:
            return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
        case ConsoleColor::GreenIntensity:
            return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case ConsoleColor::BlackIntensity:
            return 0;
        case ConsoleColor::BlueIntensity:
            return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        case ConsoleColor::GrayIntensity:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED |
                   FOREGROUND_INTENSITY;
        case ConsoleColor::PurpleIntensity:
            return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
        case ConsoleColor::RedIntensity:
            return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case ConsoleColor::WhiteIntensity:
            return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN |
                   FOREGROUND_INTENSITY;
        case ConsoleColor::YellowIntensity:
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case ConsoleColor::CyanIntensity:
            return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        default:
            return 0;
    }
}
#else
std::string getColorCode(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::Green:
            return "\033[32m";
        case ConsoleColor::Black:
            return "\033[30m";
        case ConsoleColor::Blue:
            return "\033[34m";
        case ConsoleColor::Gray:
            return "\033[37m";
        case ConsoleColor::Purple:
            return "\033[35m";
        case ConsoleColor::Red:
            return "\033[31m";
        case ConsoleColor::White:
            return "\033[37m";
        case ConsoleColor::Cyan:
            return "\033[36m";
        case ConsoleColor::Yellow:
            return "\033[33m";
        case ConsoleColor::None:
            return "\033[0m";
        case ConsoleColor::GreenIntensity:
            return "\033[32m;1m";
        case ConsoleColor::BlackIntensity:
            return "\033[30m;1m";
        case ConsoleColor::BlueIntensity:
            return "\033[34m;1m";
        case ConsoleColor::GrayIntensity:
            return "\033[37m;1m";
        case ConsoleColor::PurpleIntensity:
            return "\033[35m;1m";
        case ConsoleColor::RedIntensity:
            return "\033[31m;1m";
        case ConsoleColor::WhiteIntensity:
            return "\033[37m;1m";
        case ConsoleColor::YellowIntensity:
            return "\033[33m;1m";
        case ConsoleColor::CyanIntensity:
            return "\033[36m;1m";
        default:
            return 0;
    }
}
#endif

#if IS_WINDOWS
WORD getBackgroundColorCode(ConsoleBackgroundColor color) {
    switch (color) {
        case ConsoleBackgroundColor::Green:
            return BACKGROUND_GREEN;
        case ConsoleBackgroundColor::Black:
            return 0;
        case ConsoleBackgroundColor::Blue:
            return BACKGROUND_BLUE;
        case ConsoleBackgroundColor::Gray:
            return 0;
        case ConsoleBackgroundColor::Purple:
            return BACKGROUND_RED | BACKGROUND_BLUE;
        case ConsoleBackgroundColor::Red:
            return BACKGROUND_RED;
        case ConsoleBackgroundColor::White:
            return BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN;
        case ConsoleBackgroundColor::Cyan:
            return BACKGROUND_BLUE | BACKGROUND_GREEN;
        case ConsoleBackgroundColor::Yellow:
            return BACKGROUND_RED | BACKGROUND_GREEN;
        case ConsoleBackgroundColor::None:
            return 0;
        default:
            return 0;
    }
}
#else
std::string getBackgroundColorCode(ConsoleBackgroundColor color) {
    switch (color) {
        case ConsoleBackgroundColor::Green:
            return "\033[42m";
        case ConsoleBackgroundColor::Black:
            return "\033[40m";
        case ConsoleBackgroundColor::Blue:
            return "\033[44m";
        case ConsoleBackgroundColor::Gray:
            return "\033[40m";
        case ConsoleBackgroundColor::Purple:
            return "\033[45m";
        case ConsoleBackgroundColor::Red:
            return "\033[41m";
        case ConsoleBackgroundColor::White:
            return "\033[47m";
        case ConsoleBackgroundColor::Cyan:
            return "\033[46m";
        case ConsoleBackgroundColor::Yellow:
            return "\033[43m";
        case ConsoleBackgroundColor::None:
            return "\033[40m";
        default:
            return 0;
    }
}
#endif
std::ostream& operator<<(std::ostream& os, ConsoleColor data) {
#if IS_WINDOWS
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, getColorCode(data));
#else
    std::cout << GetColorCode(data);
#endif
    return os;
}

std::ostream& operator<<(std::ostream& os, ConsoleBackgroundColor data) {
#if IS_WINDOWS
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, getBackgroundColorCode(data));
#else
    std::cout << GetBackgroundColorCode(data);
#endif
    return os;
}
}