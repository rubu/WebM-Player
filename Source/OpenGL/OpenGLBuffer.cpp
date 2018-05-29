#include "OpenGLBuffer.h"

OpenGLBuffer::OpenGLBuffer() : buffer_(0)
{
    glGenBuffers(1, &buffer_);
    if (buffer_ == 0)
    {
        CHECK_OPENGL_CALL("glGenBuffers() failed");
    }
}

OpenGLBuffer::~OpenGLBuffer()
{
    if (buffer_)
    {
        glDeleteBuffers(1, &buffer_);
    }
}

OpenGLBuffer::operator GLuint() const
{
    return buffer_;
}
