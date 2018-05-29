#pragma once

#include "OpenGL.h"

class OpenGLShader
{
public:
    OpenGLShader(GLenum type);
    OpenGLShader(OpenGLShader&& shader);
    virtual ~OpenGLShader();
    
    operator GLuint();
    
private:
    GLuint shader_;
};

OpenGLShader compile_shader(GLenum type, const char* source);

