#pragma once

#include "VideoDecoder.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Player : VideoDecoder::IEventListener
{
    enum class Command
    {
        None,
        Pause,
        Resume,
        Stop,
    };

public:
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
        
        virtual void on_video_frame_size_changed(unsigned int width, unsigned int height) = 0;
        virtual bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */) = 0;
        virtual void on_exception(const std::exception& exception) = 0;
    };

    Player(const char* file_path, IEventListener* event_listener, bool verbose = false);
    ~Player();

    void pause();
    void start();
    void resume();
    void stop();

    // VideoDecoder::IEventListener
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], uint64_t pts /* nanoseconds */) override;

private:
    void decoding_thread();
    void execute_command(Command command);
    Command get_next_command(bool wait);

private:
    const char* file_path_;
    IEventListener* const event_listener_;
    std::atomic<bool> verbose_;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
    Command command_;
    std::thread decoding_thread_;
};
