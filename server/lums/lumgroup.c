#include "lums.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create a new LUM group
 */
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type) {
    LUMGroup* group = (LUMGroup*)malloc(sizeof(LUMGroup));
    if (!group) return NULL;
    
    group->lums = lums;
    group->count = count;
    group->group_type = type;
    group->id = NULL;
    group->connections = NULL;
    group->connection_count = 0;
    group->spatial_data = NULL;
    
    return group;
}

/**
 * Free LUM group and its resources
 */
void free_lum_group(LUMGroup* group) {
    if (!group) return;
    
    if (group->lums) {
        free(group->lums);
    }
    if (group->id) {
        free(group->id);
    }
    if (group->spatial_data) {
        free(group->spatial_data);
    }
    if (group->connections) {
        free(group->connections);
    }
    
    free(group);
}

/**
 * Clone a LUM group
 */
LUMGroup* clone_lum_group(LUMGroup* source) {
    if (!source) return NULL;
    
    LUM* cloned_lums = NULL;
    if (source->lums && source->count > 0) {
        cloned_lums = (LUM*)malloc(sizeof(LUM) * source->count);
        if (!cloned_lums) return NULL;
        memcpy(cloned_lums, source->lums, sizeof(LUM) * source->count);
    }
    
    return create_lum_group(cloned_lums, source->count, source->group_type);
}

/**
 * Compare two LUM groups
 */
int compare_lum_groups(LUMGroup* group1, LUMGroup* group2) {
    if (!group1 || !group2) return -1;
    if (group1->count != group2->count) return 1;
    
    for (size_t i = 0; i < group1->count; i++) {
        if (group1->lums[i].presence != group2->lums[i].presence) {
            return 1;
        }
    }
    
    return 0; // Equal
}

/**
 * Print LUM group for debugging
 */
void print_lum_group(LUMGroup* group) {
    if (!group) {
        printf("NULL LUMGroup\n");
        return;
    }
    
    printf("LUMGroup: %zu LUMs, type=%d\n", group->count, group->group_type);
    for (size_t i = 0; i < group->count && i < 20; i++) {
        printf("  [%zu]: %d @ (%d,%d)\n", 
               i, group->lums[i].presence, 
               group->lums[i].position.x, group->lums[i].position.y);
    }
    if (group->count > 20) {
        printf("  ... (%zu more)\n", group->count - 20);
    }
}