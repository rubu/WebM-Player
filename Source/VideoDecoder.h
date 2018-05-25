#pragma once

#include <cstddef>

class VideoDecoder
{
public:
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
        
        virtual void on_video_frame_decoded(unsigned char* (&yuv_planes)[3]) = 0;
    };
    
    VideoDecoder(IEventListener* event_listener);
    virtual ~VideoDecoder() = default;
    
    virtual void decode_i420(const unsigned char* bitstream, size_t bitstream_length) = 0;
    
protected:
    IEventListener* event_listener_;
};
