#pragma once

#include "OpenGL.h"

class OpenGLBuffer
{
public:
    OpenGLBuffer();
    virtual ~OpenGLBuffer();

    operator GLuint() const;

private:
    GLuint buffer_;
};
