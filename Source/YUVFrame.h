#pragma once

#include <cstddef>
#include <memory>

class YUVFrame
{
public:
    YUVFrame();
    YUVFrame(YUVFrame&&) = default;
    YUVFrame(size_t height, size_t y_stride, size_t u_stride, size_t v_stride);

    YUVFrame& operator=(YUVFrame&& yuv_frame) = default;
    
    bool is_empty() const;
    void load_planes(unsigned char* yuv_planes[3], size_t strides[3]);
    unsigned char* y_plane();
    unsigned char* u_plane();
    unsigned char* v_plane();

private:
    size_t height_;
    size_t y_stride_;
    size_t u_stride_;
    size_t v_stride_;
    std::unique_ptr<unsigned char[]> y_plane_;
    std::unique_ptr<unsigned char[]> u_plane_;
    std::unique_ptr<unsigned char[]> v_plane_;
};
