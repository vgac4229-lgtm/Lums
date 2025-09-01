#include "lums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h> // Required for malloc and free

// Assuming LOG_ERROR macro is defined elsewhere, e.g., in "log.h"
// For this example, let's define a simple placeholder:
#ifndef LOG_ERROR
#define LOG_ERROR(msg, ...) fprintf(stderr, "ERROR: " msg "\n", ##__VA_ARGS__)
#endif

// Declaration of create_lum_group, assuming it's defined in lums.h or a related file
// This is a placeholder, the actual implementation might differ.
// Based on the context of the provided code, it seems to be used for allocating and initializing a LUMGroup.
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type) {
    LUMGroup* result = (LUMGroup*)malloc(sizeof(LUMGroup));
    if (!result) {
        LOG_ERROR("Memory allocation failed for LUMGroup");
        return NULL;
    }
    result->lums = (LUM*)malloc(sizeof(LUM) * count);
    if (!result->lums) {
        free(result);
        LOG_ERROR("Memory allocation failed for LUMs array of size %zu", count);
        return NULL;
    }

    if (lums && count > 0) {
        memcpy(result->lums, lums, sizeof(LUM) * count);
    }
    result->count = count;
    result->group_type = type;
    result->spatial_data = NULL; // Initialize spatial_data to NULL

    return result;
}

// Declaration of free_lum_group, assuming it's defined in lums.h or a related file
void free_lum_group(LUMGroup* group) {
    if (group) {
        free(group->lums); // Free the array of LUMs
        free(group->spatial_data); // Free spatial data if allocated
        free(group);       // Free the LUMGroup structure itself
    }
}


/**
 * VORAX Operation: Fusion (⧉)
 * Combines two LUM groups into one cluster
 */
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2) {
    if (!group1 || !group2 || !group1->lums || !group2->lums) {
        LOG_ERROR("Invalid input groups for lum_fusion");
        return NULL;
    }

    size_t total_count = group1->count + group2->count;
    LUM* fused_lums = (LUM*)malloc(sizeof(LUM) * total_count);
    if (!fused_lums) {
        LOG_ERROR("Memory allocation failed for fused LUMs array");
        return NULL;
    }

    // Copy first group
    memcpy(fused_lums, group1->lums, sizeof(LUM) * group1->count);

    // Copy second group
    memcpy(fused_lums + group1->count, group2->lums, sizeof(LUM) * group2->count);

    // Update positions for spatial representation
    for (size_t i = group1->count; i < total_count; i++) {
        // This adjustment might need more context. Assuming it's for visual separation.
        fused_lums[i].position.x += group1->count * 20;
        fused_lums[i].structure_type = LUM_GROUP; // Mark as part of fusion
    }

    LUMGroup* fused_group = create_lum_group(fused_lums, total_count, GROUP_CLUSTER);
    if (!fused_group) {
        free(fused_lums); // Clean up if create_lum_group fails
        LOG_ERROR("Failed to create fused LUM group");
        return NULL;
    }
    // The caller is responsible for freeing fused_lums if create_lum_group fails.
    // However, if create_lum_group succeeds, it duplicates the data, so fused_lums should not be freed here.
    // The created fused_group will manage its memory.

    return fused_group;
}

/**
 * VORAX Operation: Split (⇅)
 * Distributes LUMs across multiple zones
 */
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count) {
    if (!source || !source->lums || zones <= 0 || !result_count) {
        if (result_count) *result_count = 0;
        LOG_ERROR("Invalid input for lum_split");
        return NULL;
    }

    LUMGroup** result = (LUMGroup**)malloc(sizeof(LUMGroup*) * zones);
    if (!result) {
        *result_count = 0;
        LOG_ERROR("Memory allocation failed for split result array");
        return NULL;
    }

    size_t lums_per_zone = source->count / zones;
    size_t remainder = source->count % zones;
    size_t current_index = 0;

    for (int i = 0; i < zones; i++) {
        size_t zone_count = lums_per_zone + ((size_t)i < remainder ? 1 : 0);

        if (zone_count == 0) {
            // Create empty zone
            result[i] = create_lum_group(NULL, 0, GROUP_LINEAR);
            if (!result[i]) {
                LOG_ERROR("Failed to create empty LUM group for zone %d", i);
                // Cleanup on failure: free already created groups and the result array
                for (int j = 0; j < i; j++) {
                    free_lum_group(result[j]);
                }
                free(result);
                *result_count = 0;
                return NULL;
            }
            continue;
        }

        LUM* zone_lums = (LUM*)malloc(sizeof(LUM) * zone_count);
        if (!zone_lums) {
            LOG_ERROR("Memory allocation failed for LUMs in zone %d", i);
            // Cleanup on failure: free already created groups and the result array
            for (int j = 0; j < i; j++) {
                free_lum_group(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }

        memcpy(zone_lums, source->lums + current_index, sizeof(LUM) * zone_count);

        // Update positions for new zone
        for (size_t j = 0; j < zone_count; j++) {
            zone_lums[j].position.x = (int)(j * 20); // Ensure cast to int if necessary
            zone_lums[j].position.y = i * 50; // Different Y for each zone
            zone_lums[j].structure_type = LUM_LINEAR;
        }

        result[i] = create_lum_group(zone_lums, zone_count, GROUP_LINEAR);
        if (!result[i]) {
            LOG_ERROR("Failed to create LUM group for zone %d", i);
            // Cleanup on failure: free allocated zone_lums, then already created groups and the result array
            free(zone_lums);
            for (int j = 0; j < i; j++) {
                free_lum_group(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        current_index += zone_count;
    }

    *result_count = zones;
    return result;
}

/**
 * VORAX Operation: Cycle (⟲)
 * Reduces LUM count by modulo operation
 */
LUMGroup* lum_cycle(LUMGroup* source, int modulo) {
    if (!source || !source->lums || modulo <= 0) {
        LOG_ERROR("Invalid input for lum_cycle");
        return NULL;
    }

    size_t result_count_raw = source->count % modulo;
    size_t final_result_count;
    LUM* cycled_lums = NULL;

    if (result_count_raw == 0) {
        // Create minimal LUM group with single presence if modulo results in zero items
        final_result_count = 1;
        cycled_lums = (LUM*)malloc(sizeof(LUM));
        if (!cycled_lums) {
            LOG_ERROR("Memory allocation failed for single LUM in lum_cycle");
            return NULL;
        }
        cycled_lums[0].presence = 1; // Default presence, might need adjustment
        cycled_lums[0].structure_type = LUM_CYCLE;
        cycled_lums[0].spatial_data = NULL;
        cycled_lums[0].position.x = 0;
        cycled_lums[0].position.y = 0;
    } else {
        final_result_count = result_count_raw;
        cycled_lums = (LUM*)malloc(sizeof(LUM) * final_result_count);
        if (!cycled_lums) {
            LOG_ERROR("Memory allocation failed for %zu LUMs in lum_cycle", final_result_count);
            return NULL;
        }
        memcpy(cycled_lums, source->lums, sizeof(LUM) * final_result_count);

        // Mark as cycle type and arrange in circular pattern
        double angle_step = 2.0 * 3.14159265359 / final_result_count;
        for (size_t i = 0; i < final_result_count; i++) {
            cycled_lums[i].structure_type = LUM_CYCLE;
            // Arrange in circle
            cycled_lums[i].position.x = (int)(50 + 30 * cos(i * angle_step));
            cycled_lums[i].position.y = (int)(50 + 30 * sin(i * angle_step));
        }
    }

    return create_lum_group(cycled_lums, final_result_count, GROUP_NODE);
}

/**
 * VORAX Operation: Flow (→)
 * Transfers LUMs to target zone (conceptual)
 */
LUMGroup* lum_flow(LUMGroup* source, const char* target_zone) {
    if (!source || !source->lums || !target_zone) {
        LOG_ERROR("Invalid input for lum_flow");
        return NULL;
    }

    // Create a copy with flow metadata
    LUM* flowed_lums = (LUM*)malloc(sizeof(LUM) * source->count);
    if (!flowed_lums) {
        LOG_ERROR("Memory allocation failed for flowed LUMs array");
        return NULL;
    }

    memcpy(flowed_lums, source->lums, sizeof(LUM) * source->count);

    // Update positions to show flow direction
    for (size_t i = 0; i < source->count; i++) {
        flowed_lums[i].position.x += 100; // Offset to show movement
        flowed_lums[i].structure_type = LUM_LINEAR; // Reset to linear for flow
    }

    LUMGroup* result = create_lum_group(flowed_lums, source->count, source->group_type);
    if (!result) {
        LOG_ERROR("Failed to create LUM group for flow operation");
        // The caller is responsible for freeing flowed_lums if create_lum_group fails.
        // However, if create_lum_group succeeds, it duplicates the data, so flowed_lums should not be freed here.
        return NULL;
    }

    // Store target zone information
    size_t target_len = strlen(target_zone);
    // Use a temporary pointer for allocation to check for success
    char* temp_spatial_data = (char*)malloc(target_len + 1);
    if (temp_spatial_data) {
        strcpy(temp_spatial_data, target_zone);
        result->spatial_data = temp_spatial_data; // Assign only if allocation and copy succeed
    } else {
        LOG_ERROR("Memory allocation failed for spatial_data in lum_flow");
        // It might be better to free the created group if spatial data is critical
        // free_lum_group(result); // Optional: depending on requirements
        // return NULL;
    }


    return result;
}

/**
 * Advanced Operation: Organic Fusion
 * Fuses groups while preserving spatial relationships
 */
LUMGroup* lum_organic_fusion(LUMGroup** groups, size_t group_count) {
    if (!groups || group_count == 0) {
        LOG_ERROR("Invalid input for lum_organic_fusion");
        return NULL;
    }

    size_t total_count = 0;
    for (size_t i = 0; i < group_count; i++) {
        if (groups[i] && groups[i]->lums) {
            total_count += groups[i]->count;
        }
    }

    if (total_count == 0) {
        LOG_ERROR("No LUMs found in any of the provided groups for organic fusion");
        return NULL;
    }

    LUM* fused_lums = (LUM*)malloc(sizeof(LUM) * total_count);
    if (!fused_lums) {
        LOG_ERROR("Memory allocation failed for fused LUMs in organic fusion");
        return NULL;
    }

    size_t current_pos = 0;
    int spiral_radius = 0;
    double angle = 0;
    const double PI = 3.14159265359;
    const double angle_increment = 0.5;
    const int radius_increment = 10;

    for (size_t i = 0; i < group_count; i++) {
        if (!groups[i] || !groups[i]->lums) continue;

        for (size_t j = 0; j < groups[i]->count; j++) {
            // Copy LUM data
            fused_lums[current_pos] = groups[i]->lums[j];

            // Arrange in spiral pattern
            fused_lums[current_pos].position.x = (int)(spiral_radius * cos(angle));
            fused_lums[current_pos].position.y = (int)(spiral_radius * sin(angle));
            fused_lums[current_pos].structure_type = LUM_GROUP; // Mark as part of fusion

            // Update spiral parameters
            angle += angle_increment;
            if (angle > 2 * PI) {
                angle = fmod(angle, 2 * PI); // Keep angle within [0, 2*PI)
                spiral_radius += radius_increment;
            }

            current_pos++;
        }
    }

    LUMGroup* fused_group = create_lum_group(fused_lums, total_count, GROUP_CLUSTER);
    if (!fused_group) {
        free(fused_lums); // Clean up if create_lum_group fails
        LOG_ERROR("Failed to create fused LUM group in organic fusion");
        return NULL;
    }
    return fused_group;
}

/**
 * Advanced Operation: Contextual Split
 * Splits based on LUM properties rather than equal distribution
 */
LUMGroup** lum_contextual_split(LUMGroup* source, size_t* result_count) {
    if (!source || !source->lums || !result_count) {
        if (result_count) *result_count = 0;
        LOG_ERROR("Invalid input for lum_contextual_split");
        return NULL;
    }

    // Count different structure types
    // Assuming LumStructureType is an enum: LUM_NODE, LUM_LINEAR, LUM_GROUP, LUM_CYCLE
    // And these map to indices 0, 1, 2, 3 respectively.
    size_t type_counts[4] = {0};
    for (size_t i = 0; i < source->count; i++) {
        // Ensure the structure_type is within the bounds of our array
        if (source->lums[i].structure_type < 4) {
            type_counts[source->lums[i].structure_type]++;
        } else {
            LOG_ERROR("Encountered unexpected structure_type %d", source->lums[i].structure_type);
            // Potentially treat as an error or assign to a default category
        }
    }

    // Count non-empty types to determine the size of the result array
    size_t active_types = 0;
    for (int i = 0; i < 4; i++) {
        if (type_counts[i] > 0) active_types++;
    }

    if (active_types == 0) {
        *result_count = 0;
        LOG_ERROR("No LUMs with known structure types found for contextual split");
        return NULL;
    }

    LUMGroup** result = (LUMGroup**)malloc(sizeof(LUMGroup*) * active_types);
    if (!result) {
        *result_count = 0;
        LOG_ERROR("Memory allocation failed for result array in contextual split");
        return NULL;
    }

    size_t group_index = 0;
    for (int type = 0; type < 4; type++) {
        if (type_counts[type] == 0) continue; // Skip types with no LUMs

        // Allocate memory for LUMs of the current type
        LUM* type_lums = (LUM*)malloc(sizeof(LUM) * type_counts[type]);
        if (!type_lums) {
            LOG_ERROR("Memory allocation failed for LUMs of type %d in contextual split", type);
            // Cleanup on failure: free allocated memory for LUMs and previously created groups
            for (size_t i = 0; i < group_index; i++) {
                free_lum_group(result[i]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }

        size_t type_index = 0;
        // Populate the type_lums array with LUMs of the current type
        for (size_t i = 0; i < source->count; i++) {
            if (source->lums[i].structure_type == (LumStructureType)type) {
                type_lums[type_index] = source->lums[i];
                // Adjust positions based on type and index within the type
                type_lums[type_index].position.x = (int)(type_index * 20);
                type_lums[type_index].position.y = type * 40; // Different Y for each type
                type_index++;
            }
        }

        // Determine the group type based on the LUM structure type
        GroupType group_type;
        if (type == LUM_GROUP) { // Assuming LUM_GROUP is mapped to index 2
            group_type = GROUP_CLUSTER;
        } else {
            group_type = GROUP_LINEAR;
        }

        result[group_index] = create_lum_group(type_lums, type_counts[type], group_type);
        if (!result[group_index]) {
            LOG_ERROR("Failed to create LUM group for type %d in contextual split", type);
            // Cleanup on failure: free allocated memory for LUMs, then previously created groups and the result array
            free(type_lums);
            for (size_t i = 0; i < group_index; i++) {
                free_lum_group(result[i]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        group_index++;
    }

    *result_count = active_types;
    return result;
}

/**
 * Free split result array
 */
void free_split_result(LUMGroup** groups, size_t count) {
    if (groups) {
        for (size_t i = 0; i < count; i++) {
            free_lum_group(groups[i]);
        }
        free(groups);
    }
}