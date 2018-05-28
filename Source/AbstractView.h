#pragma once

class IAbstractView
{
public:
    virtual ~IAbstractView() = default;
    
    virtual void resize(unsigned int width, unsigned int height) = 0;
};
