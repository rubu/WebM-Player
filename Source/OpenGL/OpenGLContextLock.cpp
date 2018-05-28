#include "OpenGLContextLock.h"

OpenGLContextLock::OpenGLContextLock(IOpenGLContext& context) : context_(context)
{
    context_.lock();
}

OpenGLContextLock::~OpenGLContextLock()
{
    context_.unlock();
}
