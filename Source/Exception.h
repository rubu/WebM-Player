#pragma once

#include "Utilities.h"

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

