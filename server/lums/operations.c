#include "lums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * VORAX Operation: Fusion (⧉)
 * Combines two LUM groups into one cluster
 */
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2) {
    if (!group1 || !group2 || !group1->lums || !group2->lums) {
        return NULL;
    }
    
    size_t total_count = group1->count + group2->count;
    LUM* fused_lums = (LUM*)malloc(sizeof(LUM) * total_count);
    if (!fused_lums) {
        return NULL;
    }
    
    // Copy first group
    memcpy(fused_lums, group1->lums, sizeof(LUM) * group1->count);
    
    // Copy second group
    memcpy(fused_lums + group1->count, group2->lums, sizeof(LUM) * group2->count);
    
    // Update positions for spatial representation
    for (size_t i = group1->count; i < total_count; i++) {
        fused_lums[i].position.x += group1->count * 20;
        fused_lums[i].structure_type = LUM_GROUP; // Mark as part of fusion
    }
    
    return create_lum_group(fused_lums, total_count, GROUP_CLUSTER);
}

/**
 * VORAX Operation: Split (⇅)
 * Distributes LUMs across multiple zones
 */
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count) {
    if (!source || !source->lums || zones <= 0 || !result_count) {
        if (result_count) *result_count = 0;
        return NULL;
    }
    
    LUMGroup** result = (LUMGroup**)malloc(sizeof(LUMGroup*) * zones);
    if (!result) {
        *result_count = 0;
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
            continue;
        }
        
        LUM* zone_lums = (LUM*)malloc(sizeof(LUM) * zone_count);
        if (!zone_lums) {
            // Cleanup on failure
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
            zone_lums[j].position.x = j * 20;
            zone_lums[j].position.y = i * 50; // Different Y for each zone
            zone_lums[j].structure_type = LUM_LINEAR;
        }
        
        result[i] = create_lum_group(zone_lums, zone_count, GROUP_LINEAR);
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
        return NULL;
    }
    
    size_t result_count = source->count % modulo;
    if (result_count == 0) {
        // Create minimal LUM group with single presence
        result_count = 1;
        LUM* cycled_lums = (LUM*)malloc(sizeof(LUM));
        if (!cycled_lums) {
            return NULL;
        }
        cycled_lums[0].presence = 1;
        cycled_lums[0].structure_type = LUM_CYCLE;
        cycled_lums[0].spatial_data = NULL;
        cycled_lums[0].position.x = 0;
        cycled_lums[0].position.y = 0;
        
        return create_lum_group(cycled_lums, result_count, GROUP_NODE);
    }
    
    LUM* cycled_lums = (LUM*)malloc(sizeof(LUM) * result_count);
    if (!cycled_lums) {
        return NULL;
    }
    
    memcpy(cycled_lums, source->lums, sizeof(LUM) * result_count);
    
    // Mark as cycle type and arrange in circular pattern
    double angle_step = 2.0 * 3.14159265359 / result_count;
    for (size_t i = 0; i < result_count; i++) {
        cycled_lums[i].structure_type = LUM_CYCLE;
        // Arrange in circle
        cycled_lums[i].position.x = (int)(50 + 30 * cos(i * angle_step));
        cycled_lums[i].position.y = (int)(50 + 30 * sin(i * angle_step));
    }
    
    return create_lum_group(cycled_lums, result_count, GROUP_NODE);
}

/**
 * VORAX Operation: Flow (→)
 * Transfers LUMs to target zone (conceptual)
 */
LUMGroup* lum_flow(LUMGroup* source, const char* target_zone) {
    if (!source || !source->lums || !target_zone) {
        return NULL;
    }
    
    // Create a copy with flow metadata
    LUM* flowed_lums = (LUM*)malloc(sizeof(LUM) * source->count);
    if (!flowed_lums) {
        return NULL;
    }
    
    memcpy(flowed_lums, source->lums, sizeof(LUM) * source->count);
    
    // Update positions to show flow direction
    for (size_t i = 0; i < source->count; i++) {
        flowed_lums[i].position.x += 100; // Offset to show movement
        flowed_lums[i].structure_type = LUM_LINEAR; // Reset to linear for flow
    }
    
    LUMGroup* result = create_lum_group(flowed_lums, source->count, source->group_type);
    if (result) {
        // Store target zone information
        size_t target_len = strlen(target_zone);
        result->spatial_data = malloc(target_len + 1);
        if (result->spatial_data) {
            strcpy((char*)result->spatial_data, target_zone);
        }
    }
    
    return result;
}

/**
 * Advanced Operation: Organic Fusion
 * Fuses groups while preserving spatial relationships
 */
LUMGroup* lum_organic_fusion(LUMGroup** groups, size_t group_count) {
    if (!groups || group_count == 0) {
        return NULL;
    }
    
    size_t total_count = 0;
    for (size_t i = 0; i < group_count; i++) {
        if (groups[i] && groups[i]->lums) {
            total_count += groups[i]->count;
        }
    }
    
    if (total_count == 0) {
        return NULL;
    }
    
    LUM* fused_lums = (LUM*)malloc(sizeof(LUM) * total_count);
    if (!fused_lums) {
        return NULL;
    }
    
    size_t current_pos = 0;
    int spiral_radius = 0;
    double angle = 0;
    
    for (size_t i = 0; i < group_count; i++) {
        if (!groups[i] || !groups[i]->lums) continue;
        
        for (size_t j = 0; j < groups[i]->count; j++) {
            fused_lums[current_pos] = groups[i]->lums[j];
            
            // Arrange in spiral pattern
            fused_lums[current_pos].position.x = (int)(spiral_radius * cos(angle));
            fused_lums[current_pos].position.y = (int)(spiral_radius * sin(angle));
            fused_lums[current_pos].structure_type = LUM_GROUP;
            
            angle += 0.5;
            if (angle > 2 * 3.14159265359) {
                angle = 0;
                spiral_radius += 10;
            }
            
            current_pos++;
        }
    }
    
    return create_lum_group(fused_lums, total_count, GROUP_CLUSTER);
}

/**
 * Advanced Operation: Contextual Split
 * Splits based on LUM properties rather than equal distribution
 */
LUMGroup** lum_contextual_split(LUMGroup* source, size_t* result_count) {
    if (!source || !source->lums || !result_count) {
        if (result_count) *result_count = 0;
        return NULL;
    }
    
    // Count different structure types
    size_t type_counts[4] = {0}; // For each LumStructureType
    for (size_t i = 0; i < source->count; i++) {
        if (source->lums[i].structure_type < 4) {
            type_counts[source->lums[i].structure_type]++;
        }
    }
    
    // Count non-empty types
    size_t active_types = 0;
    for (int i = 0; i < 4; i++) {
        if (type_counts[i] > 0) active_types++;
    }
    
    if (active_types == 0) {
        *result_count = 0;
        return NULL;
    }
    
    LUMGroup** result = (LUMGroup**)malloc(sizeof(LUMGroup*) * active_types);
    if (!result) {
        *result_count = 0;
        return NULL;
    }
    
    size_t group_index = 0;
    for (int type = 0; type < 4; type++) {
        if (type_counts[type] == 0) continue;
        
        LUM* type_lums = (LUM*)malloc(sizeof(LUM) * type_counts[type]);
        if (!type_lums) {
            // Cleanup on failure
            for (size_t i = 0; i < group_index; i++) {
                free_lum_group(result[i]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        size_t type_index = 0;
        for (size_t i = 0; i < source->count; i++) {
            if ((int)source->lums[i].structure_type == (int)type) {
                type_lums[type_index] = source->lums[i];
                type_lums[type_index].position.x = type_index * 20;
                type_lums[type_index].position.y = type * 40;
                type_index++;
            }
        }
        
        result[group_index] = create_lum_group(type_lums, type_counts[type], 
                                               type == LUM_GROUP ? GROUP_CLUSTER : GROUP_LINEAR);
        group_index++;
    }
    
    *result_count = active_types;
    return result;
}

/**
 * Memory management helper
 */
void free_lum_group(LUMGroup* group) {
    if (group) {
        if (group->lums) {
            free(group->lums);
        }
        if (group->id) {
            free(group->id);
        }
        if (group->connections) {
            free(group->connections);
        }
        if (group->spatial_data) {
            free(group->spatial_data);
        }
        free(group);
    }
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
