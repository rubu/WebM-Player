#pragma once

#include "VideoDecoder.h"

#include <aom/aom_decoder.h>

#include <memory>

class AV1VideoDecoder : public VideoDecoder
{
public:
	AV1VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener);

    // VideoDecoder
    bool decode_i420(const unsigned char* bitstream, size_t bitstream_length, uint64_t pts /* nanoseconds */) override;
    
private:
	aom_codec_ctx_t codec_context_;
};
