#include "YUVFrame.h"

YUVFrame::YUVFrame(size_t height, size_t y_stride, size_t u_stride, size_t v_stride) : height_(height),
    y_stride_(y_stride),
    u_stride_(u_stride),
    v_stride_(v_stride),
    y_plane_(new unsigned char[height_ * y_stride_]),
    u_plane_(new unsigned char[height_ * u_stride_]),
    v_plane_(new unsigned char[height_ * v_stride_])
{
}

void YUVFrame::load_planes(unsigned char* yuv_planes[3])
{
    memcpy(y_plane_.get(), yuv_planes[0], height_ * y_stride_);
    memcpy(u_plane_.get(), yuv_planes[1], height_ * u_stride_);
    memcpy(v_plane_.get(), yuv_planes[2], height_ * v_stride_);
}
