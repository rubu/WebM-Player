#pragma once

#include <cstddef>
#include <memory>

class YUVFrame
{
public:
    YUVFrame(size_t height, size_t y_stride, size_t u_stride, size_t v_stride);

    void load_planes(unsigned char* yuv_planes[3]);

private:
    const size_t height_;
    const size_t y_stride_;
    const size_t u_stride_;
    const size_t v_stride_;
    std::unique_ptr<unsigned char[]> y_plane_;
    std::unique_ptr<unsigned char[]> u_plane_;
    std::unique_ptr<unsigned char[]> v_plane_;
};
