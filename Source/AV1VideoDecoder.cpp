#include "AV1VideoDecoder.h"

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
    return true;
}
