#pragma once

#include <cstddef>
#include <cstdint>

class VideoDecoder
{
public:
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
        
        virtual bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */) = 0;
    };
    
    VideoDecoder(IEventListener* event_listener);
    virtual ~VideoDecoder() = default;
    
    virtual bool decode_i420(const unsigned char* bitstream, size_t bitstream_length, uint64_t pts /* nanoseconds */) = 0;
    
protected:
    IEventListener* event_listener_;
};
