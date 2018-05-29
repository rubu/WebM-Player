#pragma once

#include "Utilities.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>

#define CHECK_OPENGL_OBJC(exception, format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) [NSException raise:exception format:[NSString stringWithFormat"(error %d)", __VA_ARGS__, error]]; }

#endif

#define CHECK_OPENGL_CALL(format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) throw OpenGLCallError(format_message(format "(error %d)").c_str()); }


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
