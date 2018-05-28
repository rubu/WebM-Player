#pragma once

#include "OpenGL.h"

template<GLint internal_format, GLint format, GLint type>
class OpenGLTexture2D
{
public:
    OpenGLTexture2D() : initialized_(false)
    {
    }
    virtual ~OpenGLTexture2D()
    {
        if (initialized_)
        {
            glDeleteTextures(1, &texture_id_);
        }
    }
    
    void initialize(unsigned int width, unsigned int height)
    {
        glGenTextures(1, &texture_id_);
        CHECK_OPENGL_CALL("glGenTextures failed");
        initialized_ = true;
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        CHECK_OPENGL_CALL("glBindTexture(GL_TEXTURE2D, %d) failed", textured_id_);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECK_OPENGL_CALL("glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) failed");
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_OPENGL_CALL("glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) failed");
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECK_OPENGL_CALL("glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) failed");
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_OPENGL_CALL("glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) failed");
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, NULL);
        CHECK_OPENGL_CALL("glTexImage2D(GL_TEXTURE_2D, 0, %d, %u, %u, 0, %d, %d, NULL) failed", internal_format, width, height, format, type);
    }

private:
    std::atomic<bool> initialized_;
    GLuint texture_id_;
};

typedef OpenGLTexture2D<GL_RED, GL_RED, GL_UNSIGNED_BYTE> SingleChannelOpenGLTexture2D;
