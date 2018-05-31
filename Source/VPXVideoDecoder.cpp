#include "VPXVideoDecoder.h"
#include "Ebml.h"

#include <algorithm>
#include <iostream>
#include <string>

void put_frame_callback(void *user_private, const vpx_image_t *img)
{
}

static std::unique_ptr<VPXVideoDecoder> CreateVPXVideoDecoder(vpx_codec_iface_t* codec_interface, unsigned int width, unsigned int height, VideoDecoder::IEventListener* event_listener)
{
    vpx_codec_caps_t codec_capabilities = vpx_codec_get_caps(codec_interface);
    vpx_codec_ctx_t codec_context;
    vpx_codec_dec_cfg_t decoder_configuration;
    decoder_configuration.w = width;
    decoder_configuration.h = height;
    decoder_configuration.threads = 1;
    vpx_codec_err_t vpx_error = vpx_codec_dec_init(&codec_context, codec_interface, &decoder_configuration, 0);
    if (vpx_error != VPX_CODEC_OK)
    {
        throw std::runtime_error(std::string("vpx_codec_dec_init() failed with ").append(std::to_string(vpx_error)));
    }
    auto decoder = std::make_unique<VPXVideoDecoder>(codec_context, event_listener);
    if (codec_capabilities & VPX_CODEC_CAP_PUT_FRAME)
    {
        vpx_error = vpx_codec_register_put_frame_cb(&codec_context, &put_frame_callback, decoder.get());
        if (vpx_error != VPX_CODEC_OK)
        {
            throw std::runtime_error(std::string("vpx_codec_register_put_frame_cb() failed with ").append(std::to_string(vpx_error)));
        }
    }
    return decoder;
}

VPXVideoDecoder::VPXVideoDecoder(vpx_codec_ctx_t codec_context, IEventListener* event_listener) : VideoDecoder(event_listener), codec_context_(codec_context)
{
}

bool VPXVideoDecoder::decode_i420(const unsigned char* bitstream, size_t bitstream_length, uint64_t pts /* nanoseconds */)
{
    auto vpx_error = vpx_codec_decode(&codec_context_, bitstream, bitstream_length, nullptr, 0);
    if (vpx_error == VPX_CODEC_OK)
    {
        vpx_codec_iter_t iterator = nullptr;
        vpx_image_t* image = nullptr;
        while ((image = vpx_codec_get_frame(&codec_context_, &iterator)) != nullptr)
        {
            if (image->fmt == VPX_IMG_FMT_I420)
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
                throw std::runtime_error("vpx_codec_get_frame() did not return a I420 frame");
            }
        }
    }
    return true;
}

std::unique_ptr<VPXVideoDecoder> VPXVideoDecoder::CreateVP8VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener)
{
    return CreateVPXVideoDecoder(vpx_codec_vp8_dx(), width, height, event_listener);
}

std::unique_ptr<VPXVideoDecoder> VPXVideoDecoder::CreateVP9VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener)
{
    return CreateVPXVideoDecoder(vpx_codec_vp9_dx(), width, height, event_listener);
}

