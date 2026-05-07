#pragma once

#include <chrono>
#include <cstdio>
#include <format>

namespace lumina::meta {
    inline constexpr const char* Reset = "\033[0m";
    inline constexpr const char* Bold = "\033[1m";
    inline constexpr const char* Dim = "\033[2m";
    inline constexpr const char* Underline = "\033[4m";
    inline constexpr const char* Black = "\033[30m";
    inline constexpr const char* Red = "\033[31m";
    inline constexpr const char* Green = "\033[32m";
    inline constexpr const char* Yellow = "\033[33m";
    inline constexpr const char* Blue = "\033[34m";
    inline constexpr const char* Magenta = "\033[35m";
    inline constexpr const char* Cyan = "\033[36m";
    inline constexpr const char* White = "\033[37m";
    inline constexpr const char* BrightBlack = "\033[90m";
    inline constexpr const char* BrightRed = "\033[91m";
    inline constexpr const char* BrightGreen = "\033[92m";
    inline constexpr const char* BrightYellow = "\033[93m";
    inline constexpr const char* BrightBlue = "\033[94m";
    inline constexpr const char* BrightMagenta = "\033[95m";
    inline constexpr const char* BrightCyan = "\033[96m";
    inline constexpr const char* BrightWhite = "\033[97m";
    inline constexpr const char* BackBlack = "\033[40m";
    inline constexpr const char* BackRed = "\033[41m";
    inline constexpr const char* BackGreen = "\033[42m";
    inline constexpr const char* BackYellow = "\033[43m";
    inline constexpr const char* BackBlue = "\033[44m";
    inline constexpr const char* BackMagenta = "\033[45m";
    inline constexpr const char* BackCyan = "\033[46m";
    inline constexpr const char* BackWhite = "\033[47m";

    [[nodiscard]] inline std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        auto floored = std::chrono::floor<std::chrono::milliseconds>(now);

        return std::format("{}{:%Y-%m-%d %T}{}", Dim, floored, Reset);
    }

    template <typename... Args>
    void Print(std::format_string<Args...> message, Args&&... args) {
        std::string formatted = std::format(message, std::forward<Args>(args)...);
        std::printf("%s", formatted.c_str());
    }

    inline void PrintLine() {
        std::printf("\n");
    }

    template <typename... Args>
    void PrintLine(std::format_string<Args...> message, Args&&... args) {
        std::string formatted = std::format(message, std::forward<Args>(args)...);
        std::printf("%s\n", formatted.c_str());
    }

    template <typename... Args>
    void Log(std::format_string<Args...> message, Args&&... args) {
        std::string formatted = std::format(message, std::forward<Args>(args)...);

        PrintLine("{} {}{}[lumina:log]{}: {}", timestamp(), Bold, Cyan, Reset, formatted);
    }

    template <typename... Args>
    void LogError(std::format_string<Args...> message, Args&&... args) {
        std::string formatted = std::format(message, std::forward<Args>(args)...);

        PrintLine("{} {}{}[lumina:error]{}: {}", timestamp(), Bold, Red, Reset, formatted);
    }

    template <typename... Args>
    void LogDebug(bool debug, std::format_string<Args...> message, Args&&... args) {
        if (debug) {
            std::string formatted = std::format(message, std::forward<Args>(args)...);

            PrintLine("{} {}{}[lumina:debug]{}: {}", timestamp(), Bold, Magenta, Reset, formatted);
        }
    }

    template <typename... Args>
    void LogDebugNoHeader(bool debug, std::format_string<Args...> message, Args&&... args) {
        if (debug) {
            std::string formatted = std::format(message, std::forward<Args>(args)...);

            PrintLine("                                        {}", formatted);
        }
    }
}