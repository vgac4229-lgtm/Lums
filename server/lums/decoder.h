#ifndef DECODER_H
#define DECODER_H

#include "lums.h"

// Decoding functions
uint64_t decode_lum_to_bit(LUM* lums, size_t count);
uint64_t decode_lum_group(LUMGroup* group);
char* decode_to_binary_string(LUM* lums, size_t count);

// Type-specific decoding
uint8_t decode_to_uint8(LUMGroup* group);
uint16_t decode_to_uint16(LUMGroup* group);
uint32_t decode_to_uint32(LUMGroup* group);

// Array decoding
int decode_to_array(LUMGroup* group, void* output, size_t element_size, size_t max_elements);
int decode_clustered_groups(LUMGroup* group, uint64_t* values, size_t max_values);

// Validation and compression
int validate_lums(LUM* lums, size_t count);
double calculate_lum_entropy(LUMGroup* group);

#endif // DECODER_H