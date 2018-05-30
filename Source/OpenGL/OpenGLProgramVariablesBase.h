#pragma once

#include "OpenGL.h"

template<typename T>
class OpenGLProgramVariablesBase
{
public:
    template<typename U>
    class Variable
    {
    public:
        Variable() : location_(-1)
        {
        }

        GLint& location()
        {
            return location_;
        }

        U& value()
        {
            return value_;
        }
        
    private:
        GLint location_;
        U value_;
        
    };

    bool initialize(GLint opengl_program, void(*error_handler)(const char*, void*), void* user_data = nullptr)
    {
        return static_cast<T*>(this)->load_variables(opengl_program, error_handler, user_data);
    }
};

#define BEGIN_OPENGL_VARIABLES_MAP bool load_variables(GLint opengl_program, void(*error_handler)(const char*, void*), void* user_data = nullptr)\
{
#define OPENGL_ATTRIBUTE_VARIABLE(x, name) {\
    GLint& location = x.location();\
    location = glGetAttribLocation(opengl_program, name);\
    if (location < 0)\
    {\
        error_handler("failed to get location of attribute variable " name, user_data);\
        return false;\
    }\
}
#define OPENGL_ATTRIBUTE_VARIABLE_LOCATION(x, name) x = glGetAttribLocation(opengl_program, name);\
if (x < 0)\
{\
    error_handler("failed to get location of attribute variable " name, user_data);\
    return false;\
}
#define OPENGL_UNIFORM_VARIABLE(x, name) {\
    GLint& location = x.location();\
    location = glGetUniformLocation(opengl_program, name);\
    if (location < 0)\
    {\
        error_handler("failed to get location of uniform variable " name, user_data);\
        return false;\
    }\
}
#define OPENGL_UNIFORM_VARIABLE_LOCATION(x, name) x = glGetUniformLocation(opengl_program, name);\
if (x < 0)\
{\
    error_handler("failed to get location of uniform variable " name, user_data);\
    return false;\
}
#define END_OPENGL_VARIABLES_MAP return true;\
}
