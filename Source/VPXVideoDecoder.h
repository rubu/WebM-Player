#pragma once

#include "VideoDecoder.h"

#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

#include <memory>

class VPXVideoDecoder : public VideoDecoder
{
public:
	VPXVideoDecoder(vpx_codec_ctx_t codec_context, IEventListener* event_listener);

    // VideoDecoder
    void decode_i420(const unsigned char* bitstream, size_t bitstream_length, unsigned int pts) override;
    
    static std::unique_ptr<VPXVideoDecoder> CreateVP8VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener);
    static std::unique_ptr<VPXVideoDecoder> CreateVP9VideoDecoder(unsigned int width, unsigned int height, IEventListener* event_listener);
    
private:
	vpx_codec_ctx_t codec_context_;
};
