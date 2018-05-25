#pragma once

#include <cstddef>

class VideoDecoder
{
public:
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
        
        virtual void on_i420_video_frame_decoded(unsigned char* (&yuv_planes)[3], unsigned int pts) = 0;
    };
    
    VideoDecoder(IEventListener* event_listener);
    virtual ~VideoDecoder() = default;
    
    virtual void decode_i420(const unsigned char* bitstream, size_t bitstream_length, unsigned int pts) = 0;
    
protected:
    IEventListener* event_listener_;
};
