#pragma once

#include <Windows.h>

class DeviceContext
{
public:
	DeviceContext() : window_(NULL),
		device_context_(NULL)
	{
	}

	DeviceContext(HWND window) : window_(window),
		device_context_(GetDC(window))
	{
	}

	~DeviceContext()
	{
		reset();
	}

	operator HDC()
	{
		_ASSERT(device_context_ != NULL);
		return device_context_;
	}

	DeviceContext& operator =(HWND window)
	{
		*this = std::move(DeviceContext(window));
		return *this;
	}

	void reset()
	{
		if (device_context_ != NULL)
		{
			ReleaseDC(window_, device_context_);
			device_context_ = NULL;
		}
	}

private:
	HWND window_;
	HDC device_context_;
};
