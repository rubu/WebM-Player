#include "OpenGLShader.h"

OpenGLShader::OpenGLShader(GLenum type) : shader_(glCreateShader(type))
{
    if (shader_ == 0)
    {
        CHECK_OPENGL_CALL("glCreateShader(%d) failed", type);
    }
}

OpenGLShader::OpenGLShader(OpenGLShader&& shader) : shader_(shader.shader_)
{
    shader.shader_ = 0;
}

OpenGLShader::~OpenGLShader()
{
    if (shader_ != 0)
    {
        glDeleteShader(shader_);
    }
}

OpenGLShader::operator GLuint()
{
    return shader_;
}

OpenGLShader CompileShader(GLenum type, const char* source)
{
    OpenGLShader shader(type);
    glShaderSource(shader, 1, &source, NULL);
    CHECK_OPENGL_CALL("glShaderSource(%d, 1, \"%s\", NULL) failed", shader, source);
    glCompileShader(shader);
#if defined(DEBUG)
    GLint shader_compilation_log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shader_compilation_log_length);
    if (shader_compilation_log_length > 0)
    {
        std::unique_ptr<char[]> log(new char[shader_compilation_log_length]);
        glGetShaderInfoLog(shader, shader_compilation_log_length, &shader_compilation_log_length, log.get());
        printf("shader compilation log:\n%s", log.get());
    }
#endif
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        throw std::runtime_error("shader compilation failed");
    }
    return shader;
}
