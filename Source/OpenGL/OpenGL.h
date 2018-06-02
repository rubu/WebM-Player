#pragma once

#include "../Exception.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>

#define CHECK_OPENGL_OBJC(exception, format, ...) { GLenum error = glGetError(); if (error != GL_NO_ERROR) [NSException raise:exception format:[NSString stringWithFormat"(error %d)", __VA_ARGS__, error]]; }

#elif defined(_WIN32)
#include <Windows.h>
#include <gl\gl.h>

typedef char GLchar;
typedef size_t GLsizeiptr;

GLuint WINAPI glCreateShader(GLenum shader_type);
void WINAPI glDeleteShader(GLuint shader);
void WINAPI glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
void WINAPI glCompileShader(GLuint shader);
void WINAPI glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
void WINAPI glGenBuffers(GLsizei n, GLuint* buffers);
void WINAPI glDeleteBuffers(GLsizei n, const GLuint* buffers);
GLint WINAPI glGetAttribLocation(GLuint program, const GLchar *name);
GLint WINAPI glGetUniformLocation(GLuint program, const GLchar *name);
void WINAPI glUniform1i(GLint location, GLint v0);
void WINAPI glActiveTexture(GLenum texture);
GLuint WINAPI glCreateProgram(void);
void WINAPI glAttachShader(GLuint program, GLuint shader);
void WINAPI glLinkProgram(GLuint program);
void WINAPI glBindBuffer(GLenum target, GLuint buffer);
void WINAPI glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
void WINAPI glGetProgramiv(GLuint program, GLenum pname, GLint* params);
void WINAPI glUseProgram(GLuint program);
void WINAPI glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void WINAPI glUniform1f(GLint location, GLfloat v0);
void WINAPI glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
void WINAPI glEnableVertexAttribArray(GLuint index);

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
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
