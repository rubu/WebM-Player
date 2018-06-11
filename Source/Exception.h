#pragma once

#include "Utilities.h"

#include <cstdio>
#include <stdexcept>

class ExceptionBase : public std::exception
{
public:
    ExceptionBase(const char* file, unsigned int line, std::string error) : error_(std::move(error)),
        error_with_location_(format_message("%s(%d): %s", file, line, error_.c_str()))
    {
    }
    
    const char* what() const _NOEXCEPT override
    {
        return error_.c_str();
    }
    
    const char* error_with_location() const _NOEXCEPT
    {
        return error_with_location_.c_str();
    }
    
private:
    const std::string error_;
    const std::string error_with_location_;
};

#define CHECK(x, format, ...) if ((x) == false) throw ExceptionBase(__FILE__, __LINE__, format_message(format, __VA_ARGS__));

#if defined(_DEBUG) || defined(DEBUG)
#if defined(_WIN32)
#include <crtdbg.h>

#define Assert(x)  { _ASSERTE(x); }
#elif defined(__APPLE__) 
#define Assert(x) if ((x) == false) { fprintf(stderr, "expression \"%s\" at %s(%d) evaluated to false\n", #x, __FILE__, __LINE__); raise(SIGTRAP); }
#endif
#endif
