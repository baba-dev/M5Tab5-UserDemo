/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#ifndef __cplusplus
#error "app_trace.h requires C++ compilation"
#endif

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
#endif

namespace app_trace {

enum class Level : int {
    kNone    = 0,
    kError   = 1,
    kWarning = 2,
    kInfo    = 3,
    kDebug   = 4,
    kVerbose = 5,
};

constexpr Level kConfiguredLevel = static_cast<Level>(
#if defined(CONFIG_APP_LOG_LEVEL)
    CONFIG_APP_LOG_LEVEL
#else
    3
#endif
);

inline bool ShouldLog(Level level) {
    return static_cast<int>(level) <= static_cast<int>(kConfiguredLevel);
}

inline const char* ToString(Level level) {
    switch (level) {
        case Level::kError:
            return "E";
        case Level::kWarning:
            return "W";
        case Level::kInfo:
            return "I";
        case Level::kDebug:
            return "D";
        case Level::kVerbose:
            return "V";
        default:
            return "N";
    }
}

inline void Log(Level level, const char* tag, const char* fmt, ...) {
    if (!ShouldLog(level)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
#if defined(ESP_PLATFORM)
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    esp_log_level_t native_level = static_cast<esp_log_level_t>(static_cast<int>(level));
    esp_log_write(native_level, tag, "%s", buffer);
#else
    FILE* stream = (level == Level::kError || level == Level::kWarning) ? stderr : stdout;
    std::fprintf(stream, "[%s] %s: ", ToString(level), tag);
    std::vfprintf(stream, fmt, args);
    std::fprintf(stream, "\n");
    std::fflush(stream);
#endif
    va_end(args);
}

inline void Trace(const char* section, const char* event, const char* fmt = nullptr, ...) {
    if (!ShouldLog(Level::kDebug)) {
        return;
    }
    std::string message;
    if (fmt != nullptr) {
        va_list args;
        va_start(args, fmt);
        char buffer[256];
#if defined(ESP_PLATFORM)
        vsnprintf(buffer, sizeof(buffer), fmt, args);
#else
        std::vsnprintf(buffer, sizeof(buffer), fmt, args);
#endif
        va_end(args);
        message.assign(buffer);
    }
    if (!message.empty()) {
        Log(Level::kDebug, "TP", "%s:%s %s", section, event, message.c_str());
    } else {
        Log(Level::kDebug, "TP", "%s:%s", section, event);
    }
}

}  // namespace app_trace

#define APP_LOG_ERROR(tag, fmt, ...) ::app_trace::Log(::app_trace::Level::kError, tag, fmt, ##__VA_ARGS__)
#define APP_LOG_WARN(tag, fmt, ...)  ::app_trace::Log(::app_trace::Level::kWarning, tag, fmt, ##__VA_ARGS__)
#define APP_LOG_INFO(tag, fmt, ...)  ::app_trace::Log(::app_trace::Level::kInfo, tag, fmt, ##__VA_ARGS__)
#define APP_LOG_DEBUG(tag, fmt, ...) ::app_trace::Log(::app_trace::Level::kDebug, tag, fmt, ##__VA_ARGS__)
#define APP_TRACEI(tag, fmt, ...)    APP_LOG_INFO(tag, fmt, ##__VA_ARGS__)

#define APP_ASSERT(expr)                                                                                     \
    do {                                                                                                     \
        if (!(expr)) {                                                                                       \
            APP_LOG_ERROR("ASSERT", "Assertion failed: %s (%s:%d)", #expr, __FILE__, __LINE__);            \
            std::abort();                                                                                    \
        }                                                                                                    \
    } while (0)

#define APP_CHECK(expr, fmt, ...)                                                                            \
    do {                                                                                                     \
        if (!(expr)) {                                                                                       \
            APP_LOG_WARN("CHECK", fmt, ##__VA_ARGS__);                                                      \
        }                                                                                                    \
    } while (0)

#define TP(section, event, ...) ::app_trace::Trace(section, event, ##__VA_ARGS__)
