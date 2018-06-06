#include "PlayerWindow.h"
#include "resource.h"

#include <atlbase.h>
#include <atlcom.h>
#include <ShObjIdl.h>

std::string load_shader(int name)
{
	HMODULE handle = GetModuleHandle(NULL);
	HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(SHADERFILE));
	HGLOBAL data = LoadResource(handle, rc);
	auto size = SizeofResource(handle, rc);
	auto* shader = static_cast<const char*>(::LockResource(data));
	return std::string(shader, size);
}


LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* creation_parameters = reinterpret_cast<CREATESTRUCT*>(lparam);
			auto* player_window = reinterpret_cast<PlayerWindow*>(creation_parameters->lpCreateParams);
			player_window->initialize();
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		if (wparam == ID_FILE_OPEN)
		{
			auto* player_window = reinterpret_cast<PlayerWindow*>(GetWindowLongPtr(window, GWLP_USERDATA));
			player_window->open_file();
		}
		break;
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
	return 0;
}

PlayerWindow::PlayerWindow(HINSTANCE hInstance) : window_(NULL),
	menu_(NULL),
	hdc_(NULL),
	hglrc_(NULL)
{
	WNDCLASS window_class = { 0 };
	window_class.lpfnWndProc = window_procedure;
	window_class.hInstance = hInstance;
	window_class.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	window_class.lpszClassName = L"WebM Player";
	window_class.style = CS_OWNDC;
	if (RegisterClass(&window_class))
	{
		window_ = CreateWindowW(window_class.lpszClassName, L"WebM Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, this);
		SetWindowLongPtr(window_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		menu_ = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(window_, menu_);
	}
}

void PlayerWindow::initialize()
{
	try
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			24,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		hdc_ = GetDC(window_);

		int  letWindowsChooseThisPixelFormat;
		letWindowsChooseThisPixelFormat = ChoosePixelFormat(hdc_, &pfd);
		SetPixelFormat(hdc_, letWindowsChooseThisPixelFormat, &pfd);

		hglrc_ = wglCreateContext(hdc_);
		lock();
		opengl_renderer_ = std::make_unique<OpenGLRenderer>(*this, player_, load_shader(IDR_FRAGMENT_SHADER).c_str(), load_shader(IDR_VERTEX_SHADER).c_str());
	}
	catch (const ExceptionBase& exception)
	{
	}
	catch (const std::exception& exception)
	{
	}
}

void PlayerWindow::open_file()
{
	if (SUCCEEDED(CoInitialize(nullptr)))
	{
		_ATLTRY
		{
			CComPtr<IFileOpenDialog> file_open_dialog;
			file_open_dialog.CoCreateInstance(CLSID_FileOpenDialog);
			if (SUCCEEDED(file_open_dialog->Show(window_)))
			{
				CComPtr<IShellItem> item;
				if (SUCCEEDED(file_open_dialog->GetResult(&item)))
				{
					LPWSTR file_path = NULL;
					if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &file_path)))
					{
						play_file(file_path);
						CoTaskMemFree(file_path);
					}
				}
			}
		}
		_ATLCATCH(exception)
		{

		}
		_ATLCATCHALL()
		{
		}
		CoUninitialize();
	}
}

void PlayerWindow::play_file(const wchar_t* file_path)
{
	player_.start(file_path, this);
}

void PlayerWindow::lock()
{
	CHECK(wglMakeCurrent(hdc_, hglrc_), "wglMakeCurrent(0x%p, 0x%p) failed with %d", hdc_, hglrc_, GetLastError());
}

void PlayerWindow::unlock()
{
	CHECK(wglDeleteContext(hglrc_), "wglDeleteContext(0x%p, 0x%p) failed with %d", hglrc_, GetLastError());
}

bool PlayerWindow::on_video_frame_size_changed(unsigned int width, unsigned int height)
{
	return opengl_renderer_->on_video_frame_size_changed(width, height);
}

bool PlayerWindow::on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */)
{
	return opengl_renderer_->on_i420_video_frame_decoded(yuv_planes, strides, pts);
}

void PlayerWindow::on_ebml_document_ready(const EbmlDocument& ebml_document)
{

}

void PlayerWindow::on_exception(const std::exception& exception)
{

}

void PlayerWindow::set_timescale(unsigned int timescale_numerator, unsigned int timescale_denominator)
{

}