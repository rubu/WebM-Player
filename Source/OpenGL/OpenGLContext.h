#pragma once

class IOpenGLContext
{
public:
    virtual ~IOpenGLContext() = default;
    
    virtual void lock() = 0;
    virtual void unlock() = 0;
};
