#include "lums.h"
#include <stdio.h>
#include <time.h>

/**
 * Encode bits to LUMS representation
 * Each bit becomes a LUM with presence 0 or 1
 */
LUM* encode_bit_to_lum(uint64_t input, size_t bit_count) {
    if (bit_count == 0 || bit_count > 64) {
        return NULL;
    }
    
    LUM* result = (LUM*)malloc(sizeof(LUM) * bit_count);
    if (!result) {
        return NULL;
    }
    
    for (size_t i = 0; i < bit_count; i++) {
        result[i].presence = (input >> i) & 1;
        result[i].structure_type = LUM_LINEAR;
        result[i].spatial_data = NULL;
        result[i].position.x = i * 20; // Space LUMS 20 units apart
        result[i].position.y = 0;
    }
    
    return result;
}

/**
 * Create a LUM group from encoded bits
 */
LUMGroup* encode_to_lum_group(uint64_t input, size_t bit_count) {
    LUM* lums = encode_bit_to_lum(input, bit_count);
    if (!lums) {
        return NULL;
    }
    
    return create_lum_group(lums, bit_count, GROUP_LINEAR);
}

/**
 * Encode binary string to LUMS
 */
LUMGroup* encode_binary_string(const char* binary_str) {
    if (!binary_str) {
        return NULL;
    }
    
    size_t len = strlen(binary_str);
    if (len == 0) {
        return NULL;
    }
    
    LUM* lums = (LUM*)malloc(sizeof(LUM) * len);
    if (!lums) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        if (binary_str[i] == '1') {
            lums[i].presence = 1;
        } else if (binary_str[i] == '0') {
            lums[i].presence = 0;
        } else {
            free(lums);
            return NULL; // Invalid character
        }
        
        lums[i].structure_type = LUM_LINEAR;
        lums[i].spatial_data = NULL;
        lums[i].position.x = i * 20;
        lums[i].position.y = 0;
    }
    
    return create_lum_group(lums, len, GROUP_LINEAR);
}

/**
 * Encode with spatial clustering
 * Groups consecutive 1s into clusters
 */
LUMGroup* encode_with_clustering(const char* binary_str) {
    if (!binary_str) {
        return NULL;
    }
    
    size_t len = strlen(binary_str);
    if (len == 0) {
        return NULL;
    }
    
    LUM* lums = (LUM*)malloc(sizeof(LUM) * len);
    if (!lums) {
        return NULL;
    }
    
    int cluster_start = -1;
    for (size_t i = 0; i < len; i++) {
        if (binary_str[i] == '1') {
            lums[i].presence = 1;
            if (cluster_start == -1) {
                cluster_start = i;
            }
            lums[i].structure_type = LUM_GROUP;
        } else if (binary_str[i] == '0') {
            lums[i].presence = 0;
            lums[i].structure_type = LUM_LINEAR;
            cluster_start = -1;
        } else {
            free(lums);
            return NULL;
        }
        
        lums[i].spatial_data = NULL;
        lums[i].position.x = i * 20;
        lums[i].position.y = 0;
    }
    
    return create_lum_group(lums, len, GROUP_CLUSTER);
}

/**
 * Encode with different data types
 */
LUMGroup* encode_uint8(uint8_t value) {
    return encode_to_lum_group(value, 8);
}

LUMGroup* encode_uint16(uint16_t value) {
    return encode_to_lum_group(value, 16);
}

LUMGroup* encode_uint32(uint32_t value) {
    return encode_to_lum_group(value, 32);
}

/**
 * Encode array of values
 */
LUMGroup* encode_array(void* data, size_t element_size, size_t count) {
    if (!data || element_size == 0 || count == 0) {
        return NULL;
    }
    
    size_t total_bits = element_size * 8 * count;
    LUM* lums = (LUM*)malloc(sizeof(LUM) * total_bits);
    if (!lums) {
        return NULL;
    }
    
    uint8_t* bytes = (uint8_t*)data;
    size_t lum_index = 0;
    
    for (size_t i = 0; i < count * element_size; i++) {
        for (int bit = 0; bit < 8; bit++) {
            lums[lum_index].presence = (bytes[i] >> bit) & 1;
            lums[lum_index].structure_type = LUM_LINEAR;
            lums[lum_index].spatial_data = NULL;
            lums[lum_index].position.x = (lum_index % 32) * 20;
            lums[lum_index].position.y = (lum_index / 32) * 30;
            lum_index++;
        }
    }
    
    return create_lum_group(lums, total_bits, GROUP_LINEAR);
}
