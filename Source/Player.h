#pragma once

#include "VideoDecoder.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Player : VideoDecoder::IEventListener
{
public:
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
        
        virtual void on_video_frame_size_changed(unsigned int width, unsigned int height) = 0;
        virtual void on_i420_video_frame_decoded(unsigned char* (&yuv_planes)[3], unsigned int pts) = 0;
        virtual void on_exception(const std::exception& exception) = 0;
    };

    Player(const char* file_path, IEventListener* event_listener, bool verbose = false);
    ~Player();

    void start();
    void stop();

    // VideoDecoder::IEventListener
    void on_i420_video_frame_decoded(unsigned char* (&yuv_planes)[3], unsigned int pts) override;

private:
    void decoding_thread();

private:
    const char* file_path_;
    IEventListener* const event_listener_;
    std::atomic<bool> verbose_;
    std::recursive_mutex mutex_;
    std::condition_variable condition_variable_;
    std::thread decoding_thread_;
};
