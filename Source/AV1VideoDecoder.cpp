#include "AV1VideoDecoder.h"
#include "AV1.h"
#include <aom/aomdx.h>

#include <stdexcept>
#include <string>

AV1VideoDecoder::AV1VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener) : VideoDecoder(event_listener)
{
    aom_codec_iface_t* codec_interface = aom_codec_av1_dx();
    aom_codec_dec_cfg_t decoder_configuration;
    decoder_configuration.w = width;
    decoder_configuration.h = height;
    decoder_configuration.threads = 1;
    aom_codec_err_t aom_error = aom_codec_dec_init(&codec_context_, codec_interface, &decoder_configuration, 0);
    if (aom_error != AOM_CODEC_OK)
    {
        throw std::runtime_error(std::string("aom_codec_dec_init() failed with ").append(std::to_string(aom_error)));
    }
}

bool AV1VideoDecoder::decode_i420(const unsigned char* bitstream, size_t bitstream_length, uint64_t pts /* nanoseconds */)
{
    auto aom_error = aom_codec_decode(&codec_context_, bitstream, bitstream_length, nullptr);
    if (aom_error == AOM_CODEC_OK)
    {
        aom_codec_iter_t iterator = nullptr;
        aom_image_t* image = nullptr;
        while ((image = aom_codec_get_frame(&codec_context_, &iterator)) != nullptr)
        {
            if (image->fmt == AOM_IMG_FMT_I420)
            {
                unsigned char* yuv_planes[3] = {image->planes[0], image->planes[1], image->planes[2]};
                size_t strides[3] = { static_cast<size_t>(image->stride[0]), static_cast<size_t>(image->stride[1]), static_cast<size_t>(image->stride[2]) };
                if (event_listener_->on_i420_video_frame_decoded(yuv_planes, strides, pts) == false)
                {
                    return false;
                }
            }
            else
            {
                throw std::runtime_error("aom_codec_get_frame() did not return a I420 frame");
            }
        }
    }
    return true;
}
