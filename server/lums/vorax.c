#include "lums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h> // For rand()

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
    engine->last_error = NULL; // Initialize last_error to NULL

    // Initialize error_message buffer
    memset(engine->error_message, 0, sizeof(engine->error_message));

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
        vorax_set_error(engine, "Invalid engine or zone name provided.");
        return -1;
    }

    // Check if zone already exists
    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name && strcmp(engine->zones[i].name, name) == 0) {
            vorax_set_error(engine, "Zone already exists.");
            return -2; // Zone already exists
        }
    }

    // Resize zones array
    VoraxZone* new_zones = (VoraxZone*)realloc(engine->zones,
                                               sizeof(VoraxZone) * (engine->zone_count + 1));
    if (!new_zones) {
        vorax_set_error(engine, "Memory allocation failed for zones.");
        return -3; // Memory allocation failed
    }

    engine->zones = new_zones;
    VoraxZone* zone = &engine->zones[engine->zone_count];

    // Initialize zone
    zone->name = (char*)malloc(strlen(name) + 1);
    if (!zone->name) {
        vorax_set_error(engine, "Memory allocation failed for zone name.");
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
        vorax_set_error(engine, "Invalid engine or zone name provided.");
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

    vorax_set_error(engine, "Zone not found.");
    return -2; // Zone not found
}

/**
 * Get LUM group from a zone
 */
LUMGroup* vorax_get_zone_group(VoraxEngine* engine, const char* zone_name) {
    if (!engine || !zone_name) {
        vorax_set_error(engine, "Invalid engine or zone name provided.");
        return NULL;
    }

    for (size_t i = 0; i < engine->zone_count; i++) {
        if (engine->zones[i].name && strcmp(engine->zones[i].name, zone_name) == 0) {
            return engine->zones[i].group;
        }
    }

    vorax_set_error(engine, "Zone not found.");
    return NULL; // Zone not found
}

/**
 * Store LUM group in memory
 */
int vorax_store_memory(VoraxEngine* engine, const char* name, LUMGroup* group) {
    if (!engine || !name || !group) {
        vorax_set_error(engine, "Invalid engine, name, or group provided.");
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
        vorax_set_error(engine, "Memory allocation failed for new memory slot.");
        return -3;
    }

    engine->memory_slots = new_memory;
    VoraxMemory* slot = &engine->memory_slots[engine->memory_count];

    slot->name = (char*)malloc(strlen(name) + 1);
    if (!slot->name) {
        vorax_set_error(engine, "Memory allocation failed for memory slot name.");
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
        vorax_set_error(engine, "Invalid engine or name provided.");
        return NULL;
    }

    for (size_t i = 0; i < engine->memory_count; i++) {
        if (engine->memory_slots[i].name && strcmp(engine->memory_slots[i].name, name) == 0) {
            return clone_lum_group(engine->memory_slots[i].stored_group);
        }
    }

    vorax_set_error(engine, "Memory slot not found.");
    return NULL; // Memory slot not found
}

/**
 * Execute VORAX operation between zones
 */
int vorax_execute_operation(VoraxEngine* engine, const char* operation,
                           const char* source_zone, const char* target_zone,
                           void* parameters) {
    if (!engine || !operation || !source_zone) {
        vorax_set_error(engine, "Invalid engine, operation, or source zone provided.");
        return -1;
    }

    LUMGroup* source_group = vorax_get_zone_group(engine, source_zone);
    if (!source_group) {
        // Error already set by vorax_get_zone_group if zone not found
        return -2; // Source zone not found or empty
    }

    if (strcmp(operation, "fusion") == 0 || strcmp(operation, "⧉") == 0) {
        if (!target_zone) {
            vorax_set_error(engine, "Target zone not provided for fusion.");
            return -3;
        }

        LUMGroup* target_group = vorax_get_zone_group(engine, target_zone);
        if (!target_group) {
            // Error already set by vorax_get_zone_group if zone not found
            return -4;
        }

        LUMGroup* result = lum_fusion(source_group, target_group);
        if (!result) {
            vorax_set_error(engine, "Fusion operation failed.");
            return -5;
        }

        // Replace source zone with result
        vorax_set_zone_group(engine, source_zone, result);
        // Clear target zone
        vorax_set_zone_group(engine, target_zone, NULL);

    } else if (strcmp(operation, "split") == 0 || strcmp(operation, "⇅") == 0) {
        int* zones_param = (int*)parameters;
        int zones = zones_param ? *zones_param : 2;

        size_t result_count;
        LUMGroup** split_groups = lum_split(source_group, zones, &result_count);
        if (!split_groups) {
            vorax_set_error(engine, "Split operation failed.");
            return -5;
        }

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
                if (vorax_add_zone(engine, temp_name, i * 100, 0, 80, 80) != 0) {
                    // Error setting up temp zone, try to clean up allocated split_groups
                    free(split_groups[i]); // Free the group if it was allocated
                    // Potentially free remaining split_groups and return error
                    vorax_set_error(engine, "Failed to add temporary zone during split.");
                    free(split_groups);
                    return -6; // Indicate failure to add temp zone
                }
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
        if (!result) {
            vorax_set_error(engine, "Cycle operation failed.");
            return -5;
        }

        vorax_set_zone_group(engine, source_zone, result);

    } else if (strcmp(operation, "flow") == 0 || strcmp(operation, "→") == 0) {
        if (!target_zone) {
            vorax_set_error(engine, "Target zone not provided for flow.");
            return -3;
        }

        LUMGroup* result = lum_flow(source_group, target_zone);
        if (!result) {
            vorax_set_error(engine, "Flow operation failed.");
            return -5;
        }

        vorax_set_zone_group(engine, target_zone, result);
        // Clear source zone
        vorax_set_zone_group(engine, source_zone, NULL);

    } else {
        vorax_set_error(engine, "Unknown operation specified.");
        return -6; // Unknown operation
    }

    return 0; // Success
}

/**
 * Execute VORAX code string
 */
int vorax_execute_code(VoraxEngine* engine, const char* code) {
    if (!engine || !code) {
        vorax_set_error(engine, "Invalid engine or code provided.");
        return -1;
    }

    // Simple parser for basic VORAX operations
    // This is a simplified implementation

    char* code_copy = (char*)malloc(strlen(code) + 1);
    if (!code_copy) {
        vorax_set_error(engine, "Memory allocation failed for code copy.");
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
                if (vorax_add_zone(engine, zone_name, 0, 0, 100, 100) != 0) {
                    error_count++;
                    // Error message already set by vorax_add_zone
                } else {
                    // Parse LUM content after ":"
                    char* colon = strchr(line, ':');
                    if (colon) {
                        colon++;
                        while (*colon == ' ') colon++;

                        // Count LUMs (simplified)
                        int lum_count = 0;
                        for (char* p = colon; *p; p++) {
                            if (strncmp(p, "•", 3) == 0) { // Assuming '•' is 3 bytes in UTF-8
                                lum_count++;
                                p += 2; // Skip the rest of the UTF-8 bytes for '•'
                            }
                        }

                        if (lum_count > 0) {
                            // Create LUM group
                            char binary_str[65] = {0};
                            for (int i = 0; i < lum_count && i < 64; i++) {
                                binary_str[i] = '1';
                            }
                            LUMGroup* group = encode_binary_string(binary_str);
                            if (group) {
                                if (vorax_set_zone_group(engine, zone_name, group) != 0) {
                                    error_count++;
                                    free_lum_group(group); // Clean up allocated group
                                    // Error message already set by vorax_set_zone_group
                                }
                            } else {
                                error_count++;
                                vorax_set_error(engine, "Failed to encode binary string for LUM group.");
                            }
                        }
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
                printf("%s", engine->zones[i].group->lums[j].presence ? "•" : "○");
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

    const char* last_error = vorax_get_last_error(engine);
    if (last_error && last_error[0] != '\0') { // Check if error message is not empty
        printf("\nLast Error: %s\n", last_error);
    }
}

/**
 * Error handling functions
 */
const char* vorax_get_last_error(VoraxEngine* engine) {
    if (!engine) {
        return "Engine not initialized";
    }
    // Return the buffer directly, ensuring it's null-terminated
    return engine->error_message;
}

void vorax_clear_error(VoraxEngine* engine) {
    if (engine) {
        engine->error_message[0] = '\0'; // Clear the buffer
    }
}

static void vorax_set_error(VoraxEngine* engine, const char* error_msg) {
    if (engine && error_msg) {
        strncpy(engine->error_message, error_msg, sizeof(engine->error_message) - 1);
        engine->error_message[sizeof(engine->error_message) - 1] = '\0'; // Ensure null-termination
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
        // Use current time and a random number for a pseudo-unique ID
        snprintf(group->id, 32, "group_%ld_%d", time(NULL), rand() % 1000);
    } else {
        // Handle allocation failure for ID if necessary
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
        printf("%s", group->lums[i].presence ? "•" : "○");
    }
    if (group->count > 20) {
        printf("...");
    }
    printf("\n");
}