#include "lums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

/**
 * Create a new VORAX engine
 */
VoraxEngine* create_vorax_engine(void) {
    VoraxEngine* engine = (VoraxEngine*)malloc(sizeof(VoraxEngine));
    if (!engine) {
        return NULL;
    }
    
    engine->zones = NULL;
    engine->zone_count = 0;
    engine->memory_slots = NULL;
    engine->memory_count = 0;
    engine->last_error = NULL;
    
    return engine;
}

/**
 * Free VORAX engine and all its resources
 */
void free_vorax_engine(VoraxEngine* engine) {
    if (!engine) {
        return;
    }
    
    // Free zones
    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name) {
            free(engine->zones[i].name);
        }
        free_lum_group(engine->zones[i].group);
    }
    if (engine->zones) {
        free(engine->zones);
    }
    
    // Free memory slots
    for (size_t i = 0; i < engine->memory_count; i++) {
        if (engine->memory_slots[i].name) {
            free(engine->memory_slots[i].name);
        }
        free_lum_group(engine->memory_slots[i].stored_group);
    }
    if (engine->memory_slots) {
        free(engine->memory_slots);
    }
    
    // Free error message
    if (engine->last_error) {
        free(engine->last_error);
    }
    
    free(engine);
}

/**
 * Add a zone to the VORAX engine
 */
int vorax_add_zone(VoraxEngine* engine, const char* name, int x, int y, int width, int height) {
    if (!engine || !name) {
        return -1;
    }
    
    // Check if zone already exists
    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name && strcmp(engine->zones[i].name, name) == 0) {
            return -2; // Zone already exists
        }
    }
    
    // Resize zones array
    VoraxZone* new_zones = (VoraxZone*)realloc(engine->zones, 
                                               sizeof(VoraxZone) * (engine->zone_count + 1));
    if (!new_zones) {
        return -3; // Memory allocation failed
    }
    
    engine->zones = new_zones;
    VoraxZone* zone = &engine->zones[engine->zone_count];
    
    // Initialize zone
    zone->name = (char*)malloc(strlen(name) + 1);
    if (!zone->name) {
        return -3;
    }
    strcpy(zone->name, name);
    
    zone->group = NULL;
    zone->bounds.x = x;
    zone->bounds.y = y;
    zone->bounds.width = width;
    zone->bounds.height = height;
    
    engine->zone_count++;
    return 0;
}

/**
 * Set LUM group for a zone
 */
int vorax_set_zone_group(VoraxEngine* engine, const char* zone_name, LUMGroup* group) {
    if (!engine || !zone_name) {
        return -1;
    }
    
    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name && strcmp(engine->zones[i].name, zone_name) == 0) {
            // Free existing group if any
            free_lum_group(engine->zones[i].group);
            engine->zones[i].group = group;
            return 0;
        }
    }
    
    return -2; // Zone not found
}

/**
 * Get LUM group from a zone
 */
LUMGroup* vorax_get_zone_group(VoraxEngine* engine, const char* zone_name) {
    if (!engine || !zone_name) {
        return NULL;
    }
    
    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name && strcmp(engine->zones[i].name, zone_name) == 0) {
            return engine->zones[i].group;
        }
    }
    
    return NULL; // Zone not found
}

/**
 * Store LUM group in memory
 */
int vorax_store_memory(VoraxEngine* engine, const char* name, LUMGroup* group) {
    if (!engine || !name || !group) {
        return -1;
    }
    
    // Check if memory slot already exists
    for (size_t i = 0; i < engine->memory_count; i++) {
        if (engine->memory_slots[i].name && strcmp(engine->memory_slots[i].name, name) == 0) {
            // Replace existing
            free_lum_group(engine->memory_slots[i].stored_group);
            engine->memory_slots[i].stored_group = clone_lum_group(group);
            engine->memory_slots[i].timestamp = time(NULL);
            return 0;
        }
    }
    
    // Create new memory slot
    VoraxMemory* new_memory = (VoraxMemory*)realloc(engine->memory_slots,
                                                    sizeof(VoraxMemory) * (engine->memory_count + 1));
    if (!new_memory) {
        return -3;
    }
    
    engine->memory_slots = new_memory;
    VoraxMemory* slot = &engine->memory_slots[engine->memory_count];
    
    slot->name = (char*)malloc(strlen(name) + 1);
    if (!slot->name) {
        return -3;
    }
    strcpy(slot->name, name);
    
    slot->stored_group = clone_lum_group(group);
    slot->timestamp = time(NULL);
    
    engine->memory_count++;
    return 0;
}

/**
 * Retrieve LUM group from memory
 */
LUMGroup* vorax_retrieve_memory(VoraxEngine* engine, const char* name) {
    if (!engine || !name) {
        return NULL;
    }
    
    for (size_t i = 0; i < engine->memory_count; i++) {
        if (engine->memory_slots[i].name && strcmp(engine->memory_slots[i].name, name) == 0) {
            return clone_lum_group(engine->memory_slots[i].stored_group);
        }
    }
    
    return NULL; // Memory slot not found
}

/**
 * Execute VORAX operation between zones
 */
int vorax_execute_operation(VoraxEngine* engine, const char* operation, 
                           const char* source_zone, const char* target_zone, 
                           void* parameters) {
    if (!engine || !operation || !source_zone) {
        return -1;
    }
    
    LUMGroup* source_group = vorax_get_zone_group(engine, source_zone);
    if (!source_group) {
        return -2; // Source zone not found or empty
    }
    
    if (strcmp(operation, "fusion") == 0 || strcmp(operation, "⧉") == 0) {
        if (!target_zone) return -3;
        
        LUMGroup* target_group = vorax_get_zone_group(engine, target_zone);
        if (!target_group) return -4;
        
        LUMGroup* result = lum_fusion(source_group, target_group);
        if (!result) return -5;
        
        // Replace source zone with result
        vorax_set_zone_group(engine, source_zone, result);
        // Clear target zone
        vorax_set_zone_group(engine, target_zone, NULL);
        
    } else if (strcmp(operation, "split") == 0 || strcmp(operation, "⇅") == 0) {
        int* zones_param = (int*)parameters;
        int zones = zones_param ? *zones_param : 2;
        
        size_t result_count;
        LUMGroup** split_groups = lum_split(source_group, zones, &result_count);
        if (!split_groups) return -5;
        
        // Distribute results to available zones
        size_t zone_index = 0;
        for (size_t i = 0; i < result_count && zone_index < engine->zone_count; i++) {
            // Find next available zone or create temporary ones
            while (zone_index < engine->zone_count && engine->zones[zone_index].group != NULL) {
                zone_index++;
            }
            
            if (zone_index < engine->zone_count) {
                engine->zones[zone_index].group = split_groups[i];
                zone_index++;
            } else {
                // Create temporary zone
                char temp_name[32];
                snprintf(temp_name, sizeof(temp_name), "temp_%zu", i);
                vorax_add_zone(engine, temp_name, i * 100, 0, 80, 80);
                vorax_set_zone_group(engine, temp_name, split_groups[i]);
            }
        }
        
        free(split_groups);
        // Clear source zone
        vorax_set_zone_group(engine, source_zone, NULL);
        
    } else if (strcmp(operation, "cycle") == 0 || strcmp(operation, "⟲") == 0) {
        int* modulo_param = (int*)parameters;
        int modulo = modulo_param ? *modulo_param : 3;
        
        LUMGroup* result = lum_cycle(source_group, modulo);
        if (!result) return -5;
        
        vorax_set_zone_group(engine, source_zone, result);
        
    } else if (strcmp(operation, "flow") == 0 || strcmp(operation, "→") == 0) {
        if (!target_zone) return -3;
        
        LUMGroup* result = lum_flow(source_group, target_zone);
        if (!result) return -5;
        
        vorax_set_zone_group(engine, target_zone, result);
        // Clear source zone
        vorax_set_zone_group(engine, source_zone, NULL);
        
    } else {
        return -6; // Unknown operation
    }
    
    return 0; // Success
}

/**
 * Execute VORAX code string
 */
int vorax_execute_code(VoraxEngine* engine, const char* code) {
    if (!engine || !code) {
        return -1;
    }
    
    // Simple parser for basic VORAX operations
    // This is a simplified implementation
    
    char* code_copy = (char*)malloc(strlen(code) + 1);
    if (!code_copy) {
        return -2;
    }
    strcpy(code_copy, code);
    
    char* line = strtok(code_copy, "\n");
    int error_count = 0;
    
    while (line) {
        // Skip empty lines and comments
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '\0' || *line == '#') {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Parse zone declarations: "Zone A : ⦿(•••)"
        if (strncmp(line, "Zone ", 5) == 0) {
            char zone_name[32];
            if (sscanf(line, "Zone %31s :", zone_name) == 1) {
                vorax_add_zone(engine, zone_name, 0, 0, 100, 100);
                
                // Parse LUM content after ":"
                char* colon = strchr(line, ':');
                if (colon) {
                    colon++;
                    while (*colon == ' ') colon++;
                    
                    // Count LUMs (simplified)
                    int lum_count = 0;
                    for (char* p = colon; *p; p++) {
                        if (*p == '•') lum_count++;
                    }
                    
                    if (lum_count > 0) {
                        // Create LUM group
                        char binary_str[65] = {0};
                        for (int i = 0; i < lum_count && i < 64; i++) {
                            binary_str[i] = '1';
                        }
                        LUMGroup* group = encode_binary_string(binary_str);
                        vorax_set_zone_group(engine, zone_name, group);
                    }
                }
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    free(code_copy);
    return error_count;
}

/**
 * Print current state of VORAX engine
 */
void print_vorax_engine_state(VoraxEngine* engine) {
    if (!engine) {
        printf("NULL engine\n");
        return;
    }
    
    printf("VORAX Engine State:\n");
    printf("==================\n");
    
    printf("Zones (%zu):\n", engine->zone_count);
    for (size_t i = 0; i < engine->zone_count; i++) {
        printf("  %s: ", engine->zones[i].name);
        if (engine->zones[i].group) {
            printf("%zu LUMs ", engine->zones[i].group->count);
            for (size_t j = 0; j < engine->zones[i].group->count && j < 10; j++) {
                printf("%c", engine->zones[i].group->lums[j].presence ? '•' : '○');
            }
            if (engine->zones[i].group->count > 10) {
                printf("...");
            }
        } else {
            printf("(empty)");
        }
        printf("\n");
    }
    
    printf("\nMemory (%zu):\n", engine->memory_count);
    for (size_t i = 0; i < engine->memory_count; i++) {
        printf("  #%s: ", engine->memory_slots[i].name);
        if (engine->memory_slots[i].stored_group) {
            printf("%zu LUMs", engine->memory_slots[i].stored_group->count);
        } else {
            printf("(empty)");
        }
        printf("\n");
    }
    
    if (engine->last_error) {
        printf("\nLast Error: %s\n", engine->last_error);
    }
}

/**
 * Error handling functions
 */
const char* vorax_get_last_error(VoraxEngine* engine) {
    return engine ? engine->last_error : "NULL engine";
}

void vorax_clear_error(VoraxEngine* engine) {
    if (engine && engine->last_error) {
        free(engine->last_error);
        engine->last_error = NULL;
    }
}

static void vorax_set_error(VoraxEngine* engine, const char* error_msg) {
    if (!engine || !error_msg) return;
    
    if (engine->last_error) {
        free(engine->last_error);
    }
    
    engine->last_error = (char*)malloc(strlen(error_msg) + 1);
    if (engine->last_error) {
        strcpy(engine->last_error, error_msg);
    }
}

/**
 * Utility functions
 */
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type) {
    LUMGroup* group = (LUMGroup*)malloc(sizeof(LUMGroup));
    if (!group) {
        return NULL;
    }
    
    group->lums = lums;
    group->count = count;
    group->group_type = type;
    group->connections = NULL;
    group->connection_count = 0;
    group->spatial_data = NULL;
    
    // Generate unique ID
    group->id = (char*)malloc(32);
    if (group->id) {
        snprintf(group->id, 32, "group_%ld_%d", time(NULL), rand() % 1000);
    }
    
    return group;
}

LUMGroup* clone_lum_group(LUMGroup* source) {
    if (!source || !source->lums) {
        return NULL;
    }
    
    LUM* cloned_lums = (LUM*)malloc(sizeof(LUM) * source->count);
    if (!cloned_lums) {
        return NULL;
    }
    
    memcpy(cloned_lums, source->lums, sizeof(LUM) * source->count);
    
    return create_lum_group(cloned_lums, source->count, source->group_type);
}

int compare_lum_groups(LUMGroup* group1, LUMGroup* group2) {
    if (!group1 || !group2) {
        return group1 == group2 ? 0 : -1;
    }
    
    if (group1->count != group2->count) {
        return (int)(group1->count - group2->count);
    }
    
    for (size_t i = 0; i < group1->count; i++) {
        if (group1->lums[i].presence != group2->lums[i].presence) {
            return group1->lums[i].presence - group2->lums[i].presence;
        }
    }
    
    return 0; // Equal
}

void print_lum_group(LUMGroup* group) {
    if (!group) {
        printf("NULL group\n");
        return;
    }
    
    printf("LUM Group [%s]: %zu LUMs, Type: %d\n", 
           group->id ? group->id : "unknown", 
           group->count, 
           group->group_type);
    
    for (size_t i = 0; i < group->count && i < 20; i++) {
        printf("%c", group->lums[i].presence ? '•' : '○');
    }
    if (group->count > 20) {
        printf("...");
    }
    printf("\n");
}
