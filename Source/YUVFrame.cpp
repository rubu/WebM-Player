#include "YUVFrame.h"

static void load_plane(unsigned char* src, size_t src_stride, unsigned char* dst, size_t dst_stride, unsigned int height)
{
    if (src_stride == dst_stride)
    {
        memcpy(dst, src, height * dst_stride);
    }
    else
    {
        for (auto row = 0; row < height; ++row)
        {
            memcpy(dst, src, dst_stride);
            dst += dst_stride;
            src += src_stride;
        }
    }
}

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

void YUVFrame::load_planes(unsigned char* yuv_planes[3], size_t strides[3])
{
    load_plane(yuv_planes[0], strides[0], y_plane(), y_stride_, height_);
    load_plane(yuv_planes[1], strides[1], u_plane(), u_stride_, height_ / 2);
    load_plane(yuv_planes[2], strides[2], v_plane(), v_stride_, height_ / 2);
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
