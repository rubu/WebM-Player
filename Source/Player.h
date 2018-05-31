#pragma once

#include "VideoDecoder.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Player
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
        
        virtual bool on_video_frame_size_changed(unsigned int width, unsigned int height) = 0;
        virtual bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */) = 0;
        virtual void on_exception(const std::exception& exception) = 0;
    };

    Player(bool verbose = false);
    ~Player();

    void pause();
    void start(const char* file_path, IEventListener* event_listener);
    void resume();
    void stop();

private:
    void decoding_thread(const std::string& file_path, IEventListener* event_listener);
    void execute_command(Command command);
    Command get_next_command(std::unique_lock<std::mutex>& lock, bool wait);

private:
    std::atomic<bool> verbose_;
    std::mutex command_mutex_;
    std::condition_variable command_condition_variable_;
    Command command_;
    std::recursive_mutex thread_mutex_;
    std::thread decoding_thread_;
};
