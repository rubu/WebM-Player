#include "AV1.h"
#include "Endianness.h"
#include "Exception.h"

// http://av1-spec.argondesign.com/av1-spec/av1-spec.html#leb128
unsigned int leb128(const unsigned char* data, size_t available_data_length, size_t &leb128_bytes)
{
	unsigned int value = 0;
	leb128_bytes = 0;
	for (auto i = 0; i < 8; i++)
	{
		unsigned char leb128_byte = *data;
		++data;
		int leb_128_byte_value = (leb128_byte & 0x7f);
		int shifted_leb_128_byte_value = leb_128_byte_value << (i * 7);
		value |= shifted_leb_128_byte_value;
		leb128_bytes += 1;
		if (!(leb128_byte & 0x80))
		{
			break;
		}
	}
	return value;
}

size_t obu_header(const unsigned char* data, size_t frame_unit_size)
{
    size_t frame_unit_size_remaining = frame_unit_size;
	Assert((*data & 0x80) == 0); // forbidden bit should not be set
    const bool obu_extension_flag = (*data & 0x4) >> 2, obu_has_size_field = (*data & 0x2) >> 1;
	unsigned char obu_type = (*data & 0x78) >> 3;
    ++data;
    --frame_unit_size_remaining;
    if (obu_extension_flag)
    {
        ++data;
        --frame_unit_size_remaining;
    }
    if (obu_has_size_field)
    {
        size_t leb128_bytes = 0;
        unsigned int obu_size = 0;
        obu_size = leb128(data, frame_unit_size_remaining, leb128_bytes);
        return obu_size + leb128_bytes + 1 + obu_extension_flag;
    }
    else
    {
        return frame_unit_size;
    }
}

bool open_bitstream_unit(const unsigned char* data, size_t frame_unit_size)
{
    while(frame_unit_size)
    {
        auto obu_size = obu_header(data, frame_unit_size);
        Assert(obu_size <= frame_unit_size);
        if (obu_size <= frame_unit_size)
        {
            frame_unit_size -= obu_size;
            data += obu_size;
        }
        else
        {
            return false;
        }
    }
	return true;
}

bool frame_unit(const unsigned char* data, size_t frame_unit_size)
{
	while (frame_unit_size > 0)
	{
		size_t leb128_bytes = 0;
		auto obu_length = leb128(data, frame_unit_size, leb128_bytes);
		frame_unit_size -= leb128_bytes;
		data += leb128_bytes;
		Assert(obu_length <= frame_unit_size);
		if (obu_length > frame_unit_size || open_bitstream_unit(data, obu_length) == false)
		{
			return false;
		}
		frame_unit_size -= obu_length;
		data += obu_length;
	}
	return true;
}

bool temporal_unit(const unsigned char* data, size_t temporal_unit_size)
{
	while (temporal_unit_size > 0)
	{
		size_t leb128_bytes = 0;
		auto frame_unit_size = leb128(data, temporal_unit_size, leb128_bytes);
		temporal_unit_size -= leb128_bytes;
		data += leb128_bytes;
		Assert(frame_unit_size <= temporal_unit_size);
		if (frame_unit_size > temporal_unit_size || frame_unit(data, frame_unit_size) == false)
		{
			return false;
		}
		temporal_unit_size -= frame_unit_size;
		data += frame_unit_size;
	}
	return true;
}

bool parse_av1_bitstream(const unsigned char* data, size_t available_data_length)
{
    open_bitstream_unit(data, available_data_length);
	while (available_data_length > 0)
	{
		size_t leb128_bytes = 0;
		auto temporal_unit_size = leb128(data, available_data_length, leb128_bytes);
		available_data_length -= leb128_bytes;
		data += leb128_bytes;
		Assert(temporal_unit_size <= available_data_length);
		if (temporal_unit_size > available_data_length || temporal_unit(data, temporal_unit_size) == false)
		{
			return false;
		}
		temporal_unit(data, temporal_unit_size);
        printf("found temportal unit of size %u\n", temporal_unit_size);
		available_data_length -= temporal_unit_size;
		data += temporal_unit_size;
	}
	return true;
}
