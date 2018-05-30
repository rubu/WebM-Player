#include "YUVFrame.h"

YUVFrame::YUVFrame() : height_(0),
    y_stride_(0),
    u_stride_(0),
    v_stride_(0)
{
}

YUVFrame::YUVFrame(size_t height, size_t y_stride, size_t u_stride, size_t v_stride) : height_(height),
    y_stride_(y_stride),
    u_stride_(u_stride),
    v_stride_(v_stride),
    y_plane_(new unsigned char[height_ * y_stride_]),
    u_plane_(new unsigned char[height_ / 2 * u_stride_]),
    v_plane_(new unsigned char[height_ /2 * v_stride_])
{
}

YUVFrame& YUVFrame::operator=(YUVFrame&& yuv_frame)
{
    std::swap(yuv_frame.y_plane_, y_plane_);
    std::swap(yuv_frame.u_plane_, u_plane_);
    std::swap(yuv_frame.v_plane_, v_plane_);
    return *this;
}

void YUVFrame::load_planes(unsigned char* yuv_planes[3])
{
    memcpy(y_plane_.get(), yuv_planes[0], height_ * y_stride_);
    memcpy(u_plane_.get(), yuv_planes[1], height_ / 2 * u_stride_);
    memcpy(v_plane_.get(), yuv_planes[2], height_ / 2 * v_stride_);
}

unsigned char* YUVFrame::y_plane()
{
    return y_plane_.get();
}

unsigned char* YUVFrame::u_plane()
{
    return u_plane_.get();
}

unsigned char* YUVFrame::v_plane()
{
    return v_plane_.get();
}
