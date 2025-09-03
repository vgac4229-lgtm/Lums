#include "lums.h"
#include <string.h>
#include <math.h>

/**
 * Decode LUMS back to bits
 */
uint64_t decode_lum_to_bit(LUMStruct* lums, size_t count) {
    if (!lums || count == 0 || count > 64) {
        return 0;
    }

    uint64_t result = 0;

    for (size_t i = 0; i < count; i++) {
        if (lums[i].presence) {
            result |= (1ULL << i);
        }
    }

    return result;
}

/**
 * Decode LUM group to integer
 */
uint64_t decode_lum_group(LUMGroup* group) {
    if (!group || !group->lums) {
        return 0;
    }

    return decode_lum_to_bit(group->lums, group->count);
}

/**
 * Decode LUMS to binary string
 */
char* decode_to_binary_string(LUMStruct* lums, size_t count) {
    if (!lums || count == 0) {
        return NULL;
    }

    char* result = (char*)malloc(count + 1);
    if (!result) {
        return NULL;
    }

    for (size_t i = 0; i < count; i++) {
        result[i] = lums[i].presence ? '1' : '0';
    }
    result[count] = '\0';

    return result;
}

/**
 * Decode to specific data types
 */
uint8_t decode_to_uint8(LUMGroup* group) {
    if (!group || group->count > 8) {
        return 0;
    }
    return (uint8_t)decode_lum_group(group);
}

uint16_t decode_to_uint16(LUMGroup* group) {
    if (!group || group->count > 16) {
        return 0;
    }
    return (uint16_t)decode_lum_group(group);
}

uint32_t decode_to_uint32(LUMGroup* group) {
    if (!group || group->count > 32) {
        return 0;
    }
    return (uint32_t)decode_lum_group(group);
}

/**
 * Decode to array of bytes
 */
int decode_to_array(LUMGroup* group, void* output, size_t element_size, size_t max_elements) {
    if (!group || !group->lums || !output || element_size == 0) {
        return -1;
    }

    size_t bits_per_element = element_size * 8;
    size_t max_bits = max_elements * bits_per_element;
    size_t actual_bits = group->count < max_bits ? group->count : max_bits;

    uint8_t* bytes = (uint8_t*)output;
    memset(bytes, 0, max_elements * element_size);

    for (size_t i = 0; i < actual_bits; i++) {
        if (group->lums[i].presence) {
            size_t byte_index = i / 8;
            size_t bit_index = i % 8;
            bytes[byte_index] |= (1 << bit_index);
        }
    }

    return (int)(actual_bits / bits_per_element);
}

/**
 * Decode clustered LUMS
 * Interprets grouped LUMs as separate values
 */
int decode_clustered_groups(LUMGroup* group, uint64_t* values, size_t max_values) {
    if (!group || !group->lums || !values || max_values == 0) {
        return -1;
    }

    size_t value_count = 0;
    size_t cluster_start = 0;
    uint64_t current_value = 0;

    for (size_t i = 0; i <= group->count && value_count < max_values; i++) {
        // End of cluster or end of array
        if (i == group->count || 
            (i > 0 && group->lums[i].structure_type != group->lums[i-1].structure_type)) {

            if (i > cluster_start) {
                // Decode the cluster
                current_value = 0;
                for (size_t j = cluster_start; j < i; j++) {
                    if (group->lums[j].presence) {
                        current_value |= (1ULL << (j - cluster_start));
                    }
                }
                values[value_count++] = current_value;
            }
            cluster_start = i;
        }
    }

    return (int)value_count;
}

/**
 * Validate LUM structure integrity
 */
int validate_lums(LUMStruct* lums, size_t count) {
    if (!lums) {
        return 0;
    }

    for (size_t i = 0; i < count; i++) {
        // Validation stricte de la présence
        if (lums[i].presence != 0 && lums[i].presence != 1) {
            return 0; // Invalid presence value - must be exactly 0 or 1
        }
        
        // Validation du type de structure
        if (lums[i].structure_type < LUM_LINEAR || lums[i].structure_type > LUM_CYCLE) {
            return 0; // Invalid structure type
        }
        
        // Validation des positions spatiales
        if (lums[i].position.x < -10000 || lums[i].position.x > 10000 ||
            lums[i].position.y < -10000 || lums[i].position.y > 10000) {
            return 0; // Position out of valid range
        }
        
        // Validation de cohérence : vérifier les contraintes entre LUMs adjacents
        if (i > 0) {
            // Distance minimale entre LUMs pour éviter les overlaps
            int dx = lums[i].position.x - lums[i-1].position.x;
            int dy = lums[i].position.y - lums[i-1].position.y;
            int distance_sq = dx*dx + dy*dy;
            
            if (distance_sq < 100) { // Distance minimale de 10 unités
                return 0; // LUMs too close together
            }
        }
    }

    return 1; // Valid
}

/**
 * Calculate information entropy of LUM group
 */
double calculate_lum_entropy(LUMGroup* group) {
    if (!group || !group->lums || group->count == 0) {
        return 0.0;
    }

    size_t ones = 0;
    for (size_t i = 0; i < group->count; i++) {
        if (group->lums[i].presence) {
            ones++;
        }
    }

    if (ones == 0 || ones == group->count) {
        return 0.0; // No entropy in uniform data
    }

    double p1 = (double)ones / group->count;
    double p0 = 1.0 - p1;

    return -(p1 * log2(p1) + p0 * log2(p0));
}

/**
 * Compress LUMS using run-length encoding
 */
typedef struct {
    uint8_t value;
    size_t count;
} LumRun;

int compress_lums_rle(LUMGroup* group, LumRun* runs, size_t max_runs) {
    if (!group || !group->lums || !runs || max_runs == 0) {
        return -1;
    }

    size_t run_count = 0;
    uint8_t current_value = group->lums[0].presence;
    size_t current_count = 1;

    for (size_t i = 1; i < group->count && run_count < max_runs; i++) {
        if (group->lums[i].presence == current_value) {
            current_count++;
        } else {
            runs[run_count].value = current_value;
            runs[run_count].count = current_count;
            run_count++;

            current_value = group->lums[i].presence;
            current_count = 1;
        }
    }

    // Add final run
    if (run_count < max_runs) {
        runs[run_count].value = current_value;
        runs[run_count].count = current_count;
        run_count++;
    }

    return (int)run_count;
}