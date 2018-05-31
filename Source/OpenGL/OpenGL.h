#pragma once

#include "../Exception.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>

#define CHECK_OPENGL_OBJC(exception, format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) [NSException raise:exception format:[NSString stringWithFormat"(error %d)", __VA_ARGS__, error]]; }

#endif

#define CHECK_OPENGL(format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) throw OpenGLCallError(__FILE__, __LINE__, format_message(format "(error %d)", ##__VA_ARGS__).c_str()); }
#if defined(_DEBUG) || defined(DEBUG)
#define CHECK_OPENGL_DEBUG(format, ...) CHECK_OPENGL(format, ##__VA_ARGS__)
#else
#define CHECK_OPENGL_DEBUG(format, ...)
#endif

class OpenGLCallError : public ExceptionBase
{
public:
    using ExceptionBase::ExceptionBase;
};

typedef struct
{
    float x,y,z;
    float s0, t0;
} OpenGLVertexInfo;
