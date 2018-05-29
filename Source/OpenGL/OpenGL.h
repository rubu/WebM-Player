#pragma once

#include "Utilities.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>

#define CHECK_OPENGL_OBJC(exception, format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) [NSException raise:exception format:[NSString stringWithFormat"(error %d)", __VA_ARGS__, error]]; }

#endif

#define CHECK_OPENGL(format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) throw OpenGLCallError(format_message(format "(error %d)", ##__VA_ARGS__).c_str()); }
#if defined(_DEBUG) || defined(DEBUG)
#define CHECK_OPENGL_DEBUG(format, ...) CHECK_OPENGL(format, ##__VA_ARGS__)
#else
#define CHECK_OPENGL_DEBUG(format, ...)
#endif

class OpenGLCallError : public std::exception
{
public:
    OpenGLCallError(std::string error) : error_(std::move(error))
    {
    }
    
    const char* what() const _NOEXCEPT override
    {
        return error_.c_str();
    }

private:
    const std::string error_;
};

typedef struct
{
    float x,y,z;
    float s0, t0;
} OpenGLVertexInfo;
