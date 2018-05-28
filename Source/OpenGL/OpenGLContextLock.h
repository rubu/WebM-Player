#include "OpenGLContext.h"

class OpenGLContextLock
{
public:
    OpenGLContextLock(IOpenGLContext& context);
    ~OpenGLContextLock();

private:
    IOpenGLContext& context_;
};
