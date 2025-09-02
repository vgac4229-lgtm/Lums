#ifndef ENCODER_H
#define ENCODER_H

#include "lums.h"

// Encoding functions
LUM* encode_bit_to_lum(uint64_t input, size_t bit_count);
LUMGroup* encode_to_lum_group(uint64_t input, size_t bit_count);
LUMGroup* encode_binary_string(const char* binary_str);
LUMGroup* encode_with_clustering(const char* binary_str);

// Type-specific encoding
LUMGroup* encode_uint8(uint8_t value);
LUMGroup* encode_uint16(uint16_t value);
LUMGroup* encode_uint32(uint32_t value);

// Array encoding
LUMGroup* encode_array(void* data, size_t element_size, size_t count);

#endif // ENCODER_H