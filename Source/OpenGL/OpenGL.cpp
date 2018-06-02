#include "OpenGL.h"

#if defined(_WIN32)
#define IMPLEMENT_FUNCTION(x) using FunctionType = decltype(&x); static FunctionType implementation = reinterpret_cast<decltype(&x)>(wglGetProcAddress(#x));

GLuint WINAPI glCreateShader(GLenum shader_type)
{
	IMPLEMENT_FUNCTION(glCreateShader);
	return implementation(shader_type);
}

void WINAPI glDeleteShader(GLuint shader)
{
	IMPLEMENT_FUNCTION(glDeleteShader);
	implementation(shader);
}

void WINAPI glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	IMPLEMENT_FUNCTION(glShaderSource);
	implementation(shader, count, string, length);
}

void WINAPI glCompileShader(GLuint shader)
{
	IMPLEMENT_FUNCTION(glCompileShader);
	implementation(shader);
}

void WINAPI glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	IMPLEMENT_FUNCTION(glGetShaderiv);
	implementation(shader, pname, params);
}

void WINAPI glGenBuffers(GLsizei n, GLuint* buffers)
{
	IMPLEMENT_FUNCTION(glGenBuffers);
	implementation(n, buffers);
}

void WINAPI glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	IMPLEMENT_FUNCTION(glDeleteBuffers);
	implementation(n, buffers);
}

GLint WINAPI glGetAttribLocation(GLuint program, const GLchar *name)
{
	IMPLEMENT_FUNCTION(glGetAttribLocation);
	return implementation(program, name);
}

GLint WINAPI glGetUniformLocation(GLuint program, const GLchar *name)
{
	IMPLEMENT_FUNCTION(glGetUniformLocation);
	return implementation(program, name);
}

void WINAPI glUniform1i(GLint location, GLint v0)
{
	IMPLEMENT_FUNCTION(glUniform1i);
	implementation(location ,v0);
}

void WINAPI glActiveTexture(GLenum texture)
{
	IMPLEMENT_FUNCTION(glActiveTexture);
	implementation(texture);
}

GLuint WINAPI glCreateProgram(void)
{
	IMPLEMENT_FUNCTION(glCreateProgram);
	return implementation();
}

void WINAPI glAttachShader(GLuint program, GLuint shader)
{
	IMPLEMENT_FUNCTION(glAttachShader);
	implementation(program, shader);
}

void WINAPI glLinkProgram(GLuint program)
{
	IMPLEMENT_FUNCTION(glLinkProgram);
	implementation(program);
}

void WINAPI glBindBuffer(GLenum target, GLuint buffer)
{
	IMPLEMENT_FUNCTION(glBindBuffer);
	implementation(target, buffer);
}

void WINAPI glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage)
{
	IMPLEMENT_FUNCTION(glBufferData);
	implementation(target, size, data, usage);
}

void WINAPI glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	IMPLEMENT_FUNCTION(glGetProgramiv);
	implementation(program, pname, params);
}

void WINAPI glUseProgram(GLuint program)
{
	IMPLEMENT_FUNCTION(glUseProgram);
	implementation(program);
}

void WINAPI glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	IMPLEMENT_FUNCTION(glUniformMatrix4fv);
	implementation(location, count, transpose, value);
}

void WINAPI glUniform1f(GLint location, GLfloat v0)
{
	IMPLEMENT_FUNCTION(glUniform1f);
	implementation(location, v0);
}

void WINAPI glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
	IMPLEMENT_FUNCTION(glVertexAttribPointer);
	implementation(index, size, type, normalized, stride, pointer);
}

void WINAPI glEnableVertexAttribArray(GLuint index)
{
	IMPLEMENT_FUNCTION(glEnableVertexAttribArray);
	implementation(index);
}

#endif