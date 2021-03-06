#include "Player.h"
#include "EbmlParser.h"
#if defined(__APPLE__)
#define HAVE_VPX
#define HAVE_AV1
#elif defined(_WIN32)
#define HAVE_VPX
#endif
#if defined(HAVE_VPX)
#include "VPXVideoDecoder.h"
#endif
#if defined(HAVE_AV1)
#include "AV1VideoDecoder.h"
#endif

#include <algorithm>
#if defined(_WIN32)
#include <WinSock2.h>
#endif

class VideoDecoderDelegate : public VideoDecoder::IEventListener
{
public:
    VideoDecoderDelegate(Player::IEventListener* event_listener) : event_listener_(event_listener)
    {
        
    }
    
    // VideoDecoder::IEventListener
    bool on_i420_video_frame_decoded(unsigned char* yuv_planes[3], size_t strides[3], uint64_t pts /* nanoseconds */)
    {
        return event_listener_->on_i420_video_frame_decoded(yuv_planes, strides, pts);
    }

private:
    Player::IEventListener* const event_listener_;
};

enum class VideoCodecType
{
	Unknown,
	VP8,
	VP9,
	AV1
};

VideoCodecType video_codec_type_from_string(const std::string& video_codec_id_value)
{
#if defined(HAVE_VPX)
	if (video_codec_id_value == "V_VP8")
	{
		return VideoCodecType::VP8;
	}
	if (video_codec_id_value == "V_VP9")
	{
		return VideoCodecType::VP9;
	}
#endif
#if defined(HAVE_AV1)
	if (video_codec_id_value == "V_AV1")
	{
		return VideoCodecType::AV1;
	}
#endif
	return VideoCodecType::Unknown;
}

Player::Player(bool verbose) : verbose_(verbose),
    command_(Command::None)
{
}

Player::~Player()
{
    stop();
}

#if defined(_WIN32)
void Player::start(const wchar_t* file_path, IEventListener* event_listener)
#else
void Player::start(const char* file_path, IEventListener* event_listener)
#endif
{
    std::lock_guard<std::recursive_mutex> lock(thread_mutex_);
    stop();
    decoding_thread_ = std::thread(std::bind(&Player::decoding_thread, this, file_path, event_listener));
}

void Player::stop()
{
    if (decoding_thread_.joinable())
    {
        execute_command(Command::Stop);
        decoding_thread_.join();
    }
}

#if defined(_WIN32)
void Player::decoding_thread(const std::wstring& file_path, IEventListener* event_listener)
#else
void Player::decoding_thread(const std::string& file_path, IEventListener* event_listener)
#endif
{
    try
    {
        std::unique_lock<std::mutex> lock(command_mutex_);
        auto ebml_document = parse_ebml_file(file_path.c_str());
        event_listener->on_ebml_document_ready(ebml_document);
        const auto& ebml_element_tree = ebml_document.elements();
        if (ebml_element_tree.empty())
        {
            return;
        }
        auto segment = std::find_if(ebml_element_tree.begin(), ebml_element_tree.end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Segment; });
        if (segment == ebml_element_tree.end())
        {
            throw std::runtime_error("No Segment element present");
        }
        auto info = std::find_if(segment->children().begin(), segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Info; });
        if (info == segment->children().end())
        {
            throw std::runtime_error("no Info element present");
        }
        auto timecode_scale = std::find_if(info->children().begin(), info->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TimecodeScale; });
        if (timecode_scale == info->children().end())
        {
            throw std::runtime_error("Info does not contain a TimecodeScale element");
        }
        event_listener->set_timescale(std::stoi(timecode_scale->value()), 1000000000);
        auto tracks = std::find_if(segment->children().begin(), segment->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Tracks; });
        if (tracks == segment->children().end())
        {
            throw std::runtime_error("no Tracks element present");
        }
        auto track_entry = std::find_if(tracks->children().begin(), tracks->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackEntry; });
        std::unique_ptr<VideoDecoder> video_decoder;
        unsigned int video_track_number = 0;
        VideoDecoderDelegate video_decoder_delegate(event_listener);
        while (track_entry != tracks->children().end())
        {
            auto track_number = std::find_if(track_entry->children().begin(), track_entry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackNumber; });
            if (track_number == track_entry->children().end())
            {
                throw std::runtime_error("TrackEntry does not contain a TrackNumber element");
            }
            video_track_number = std::stoi(track_number->value());
            auto codec_id = std::find_if(track_entry->children().begin(), track_entry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::CodecID; });
            if (codec_id != track_entry->children().end())
            {
                auto codec_id_value = codec_id->value();
                auto video = std::find_if(track_entry->children().begin(), track_entry->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Video; });
                if (video == (track_entry->children().end()))
                {
                    throw std::runtime_error("TrackEntry with codec " + codec_id_value + " does not contain a Video element");
                }
                auto pixel_width = std::find_if(video->children().begin(), video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelWidth; });
                if (pixel_width == (video->children().end()))
                {
                    throw std::runtime_error("Video element does not contain a PixelWidth child element");
                }
                auto pixel_height = std::find_if(video->children().begin(), video->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::PixelHeight; });
                if (pixel_height == (video->children().end()))
                {
                    throw std::runtime_error("Video element does not contain a PixelHeight child element");
                }
                unsigned int width = std::stoi(pixel_width->value()), height = std::stoi(pixel_height->value());
				auto video_codec_type = video_codec_type_from_string(codec_id_value);
				switch(video_codec_type)
				{
#if defined(HAVE_VPX)
				case VideoCodecType::VP8:
					video_decoder = VPXVideoDecoder::CreateVP8VideoDecoder(width, height, &video_decoder_delegate);
					break;
				case VideoCodecType::VP9:
					video_decoder = VPXVideoDecoder::CreateVP8VideoDecoder(width, height, &video_decoder_delegate);
					break;
#endif
#if defined(HAVE_AV1)
				case VideoCodecType::AV1:
					video_decoder = std::make_unique<AV1VideoDecoder>(width, height, &video_decoder_delegate);
					break;
#endif
				}
                if (event_listener->on_video_frame_size_changed(width, height) == false)
                {
                    return;
                }
                break;
            }
            track_entry = std::find_if(++track_entry, tracks->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::TrackEntry; });
        }
        if (track_entry == tracks->children().end())
        {
            throw std::runtime_error("no video track found");
        }
        auto clusters = segment->descendants(EbmlElementId::Cluster);
        char timestamp[13] = { 0 };
        for(const auto& cluster : clusters)
        {
            auto timecode = std::find_if(cluster->children().begin(), cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::Timecode; });
            if (timecode == (cluster->children().end()))
            {
                throw std::runtime_error("Cluster element does not contain a Timecode child element");
            }
            auto cluster_pts = std::stoi(timecode->value());
            auto simple_block = std::find_if(cluster->children().begin(), cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::SimpleBlock; });
            while (simple_block != cluster->children().end())
            {
                auto* data = simple_block->data();
                auto size = simple_block->size();
                size_t track_number_size_length;
                auto track_number = get_ebml_element_size(data, static_cast<size_t>(size), track_number_size_length);
                unsigned char flags = *(data + track_number_size_length + 2);
                if (track_number == video_track_number)
                {
                    unsigned char flags = *(data + track_number_size_length + 2);
                    if ((flags & 0x6e) != 0)
                    {
                        throw std::runtime_error("lacing is not supported");
                    }
                    auto pts = cluster_pts + ntohs(*reinterpret_cast<const short*>(data + track_number_size_length));
                    if (verbose_)
                    {
                        auto milliseconds = pts % 1000;
                        auto seconds = ((pts - milliseconds) % 60000) / 1000;
                        auto minutes = ((pts - milliseconds - seconds * 1000) % 3600000) / 60000;
                        auto hours = (pts - milliseconds - seconds * 1000 - minutes * 3600000) / 3600000;
#if defined(_WIN32)
						sprintf_s(timestamp, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
#else
                        sprintf(timestamp, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
#endif
                        std::cout << "frame @ " << timestamp << ", size - " << size - track_number_size_length - 3 << std::endl;
                    }
                    bool wait = video_decoder->decode_i420(data + track_number_size_length + 3, size - track_number_size_length - 3, pts) == false;
                    switch (get_next_command(lock, wait))
                    {
                        case Command::Stop:
                            return;
                        case Command::Pause:
                            {
                                Command command;
                                while ((command = get_next_command(lock, true)) == Command::Pause)
                                {
                                }
                                if(command == Command::Stop)
                                {
                                    return;
                                }
                            }
                            break;
                        case Command::Resume:
                        case Command::None:
                            break;
                    }
                }
                simple_block = std::find_if(++simple_block, cluster->children().end(), [](const EbmlElement& ebml_element) { return ebml_element.id() == EbmlElementId::SimpleBlock; });
            }
        }
    }
    catch (const std::exception& exception)
    {
        event_listener->on_exception(exception);
    }
}

void Player::execute_command(Command command)
{
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        command_ = command;
    }
    command_condition_variable_.notify_one();
}

Player::Command Player::get_next_command(std::unique_lock<std::mutex>& lock, bool wait)
{
    Command command = Command::None;
    if (command_ == Command::None && wait)
    {
        command_condition_variable_.wait(lock);
    }
    std::swap(command_, command);
    return command;
}

void Player::resume()
{
    execute_command(Command::Resume);
}
