#pragma once

#include "../OpenGLRenderer.h"
#include "../Player.h"
#include "../OpenGL/OpenGLContext.h"

#include <Windows.h>

class PlayerWindow : public IOpenGLContext, public Player::IEventListener
{
public:
	PlayerWindow(HINSTANCE hInstance);

	void initialize();
	void open_file();
	void play_file(const wchar_t* file_path);

	// IOpenGLContext
	void lock() override;
	void unlock() override;

	// Player::IEventListener
	bool on_video_frame_size_changed(unsigned int width, unsigned int height) override;
	bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */) override;
	void on_ebml_document_ready(const EbmlDocument& ebml_document) override;
	void on_exception(const std::exception& exception) override;
	void set_timescale(unsigned int timescale_numerator, unsigned int timescale_denominator) override;

private:
	HWND window_;
	HMENU menu_;
	Player player_;
	std::unique_ptr<OpenGLRenderer> opengl_renderer_;
	HDC hdc_;
	HGLRC hglrc_;
};