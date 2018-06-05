#pragma once

#include <string>

#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

static inline std::string format_message(const char* format, ...)
{
    std::string formatted_message;
    va_list variable_argument_list;
    va_start(variable_argument_list, format);
#if defined(_WIN32)
    const auto formatted_size = _vscprintf(format, variable_argument_list) + 1;
    formatted_message.resize(formatted_size);
    vsnprintf_s(const_cast<char*>(formatted_message.data()), formatted_message.size(), formatted_message.size(), format, variable_argument_list);
#else
    va_list variable_argument_list_copy;
    va_copy(variable_argument_list_copy, variable_argument_list);
#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    auto formatted_size = vsnprintf(NULL, 0, format, variable_argument_list_copy);
    if (formatted_size <= 0)
    {
        return "";
    }
    formatted_message.resize(static_cast<size_t>(formatted_size) + 1);
    vsnprintf(const_cast<char*>(formatted_message.data()), formatted_message.size(), format, variable_argument_list);
#if defined(__APPLE__)
#pragma clang diagnostic pop
#endif
#endif
    va_end(variable_argument_list);
    return formatted_message;
}

#if defined(__APPLE__)
static uint64_t get_host_time(uint64_t absolute_time = mach_absolute_time())
{
    static mach_timebase_info_data_t timebase_info {0, 0};
    if (timebase_info.denom == 0)
    {
        mach_timebase_info(&timebase_info);
    }
    return absolute_time * timebase_info.numer / timebase_info.denom;
}

static const unsigned int host_timescale = 1000000000;

#endif
