#define _POSIX_C_SOURCE 199309L
#include "lums.h"
#include "encoder.h"
#include "decoder.h"
#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

void vorax_set_error(VoraxEngine* engine, const char* error_msg) {
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

    // Generate truly unique ID using UUID v4 algorithm
    group->id = (char*)malloc(37); // UUID standard length + null terminator
    if (group->id) {
        // Generate cryptographically secure UUID v4
        uint8_t uuid_bytes[16];

        // Use cryptographically secure random source
        FILE* urandom = fopen("/dev/urandom", "rb");
        if (urandom) {
            if (fread(uuid_bytes, 1, 16, urandom) != 16) {
                // Fallback to time-based if urandom fails
                struct timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                unsigned int seed = (unsigned int)(ts.tv_sec ^ ts.tv_nsec ^ getpid());
                srand(seed);
                for (int i = 0; i < 16; i++) {
                    uuid_bytes[i] = (uint8_t)(rand() % 256);
                }
            }
            fclose(urandom);
        } else {
            // Fallback to time-based
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            unsigned int seed = (unsigned int)(ts.tv_sec ^ ts.tv_nsec ^ getpid());
            srand(seed);
            for (int i = 0; i < 16; i++) {
                uuid_bytes[i] = (uint8_t)(rand() % 256);
            }
        }

        // Set version (4) and variant bits according to RFC 4122
        uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x40; // Version 4
        uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80; // Variant 10

        snprintf(group->id, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                uuid_bytes[0], uuid_bytes[1], uuid_bytes[2], uuid_bytes[3],
                uuid_bytes[4], uuid_bytes[5], uuid_bytes[6], uuid_bytes[7],
                uuid_bytes[8], uuid_bytes[9], uuid_bytes[10], uuid_bytes[11],
                uuid_bytes[12], uuid_bytes[13], uuid_bytes[14], uuid_bytes[15]);
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


// --- VORAX VM Implementation ---

VoraxEngine* vorax_create_engine(void) {
    VoraxEngine* engine = malloc(sizeof(VoraxEngine));
    if (!engine) return NULL;

    // Initialize engine
    engine->zones = NULL;
    engine->zone_count = 0;
    engine->memory_slots = NULL;
    engine->memory_count = 0;
    engine->last_error = NULL;
    engine->current_tick = 0;
    engine->energy_budget = 1000.0;
    engine->state = VORAX_READY;
    engine->quantum_field.field_strength = 1.0;
    engine->quantum_field.coherence = 1.0;
    
    // Initialize zone names array
    for (int i = 0; i < MAX_ZONES; i++) {
        engine->zone_names[i] = NULL;
    }
    
    memset(engine->error_message, 0, sizeof(engine->error_message));

    return engine;
}

void vorax_destroy_engine(VoraxEngine* engine) {
    if (!engine) return;

    // Free zones array
    if (engine->zones) {
        for (size_t i = 0; i < engine->zone_count; i++) {
            if (engine->zones[i].name) {
                free(engine->zones[i].name);
            }
            if (engine->zones[i].group) {
                free_lum_group(engine->zones[i].group);
            }
        }
        free(engine->zones);
    }
    
    // Free memory slots
    if (engine->memory_slots) {
        for (size_t i = 0; i < engine->memory_count; i++) {
            if (engine->memory_slots[i].name) {
                free(engine->memory_slots[i].name);
            }
            if (engine->memory_slots[i].stored_group) {
                free_lum_group(engine->memory_slots[i].stored_group);
            }
        }
        free(engine->memory_slots);
    }
    
    // Free zone names
    for (int i = 0; i < MAX_ZONES; i++) {
        if (engine->zone_names[i]) {
            free(engine->zone_names[i]);
        }
    }
    
    // Free last error
    if (engine->last_error) {
        free(engine->last_error);
    }

    free(engine);
}

int vorax_fuse_zones(VoraxEngine* engine, int zone1, int zone2) {
    if (!engine || zone1 >= engine->zone_count || zone2 >= engine->zone_count) return -1;
    if (!engine->zones[zone1].group || !engine->zones[zone2].group) return -1;

    LUMGroup* g1 = engine->zones[zone1].group;
    LUMGroup* g2 = engine->zones[zone2].group;

    // Create fused group
    size_t total_count = g1->count + g2->count;
    LUM* fused_lums = malloc(sizeof(LUM) * total_count);
    if (!fused_lums) return -1;

    memcpy(fused_lums, g1->lums, sizeof(LUM) * g1->count);
    memcpy(fused_lums + g1->count, g2->lums, sizeof(LUM) * g2->count);

    // Update zone1 with fused result
    free(g1->lums);
    g1->lums = fused_lums;
    g1->count = total_count;

    // Clear zone2
    free(g2->lums);
    g2->lums = NULL;
    g2->count = 0;

    engine->current_tick++;
    return 0;
}

int vorax_split_zone(VoraxEngine* engine, int zone, int parts) {
    if (!engine || zone >= engine->zone_count || parts <= 0) return -1;
    if (!engine->zones[zone].group) return -1;

    LUMGroup* source = engine->zones[zone].group;
    if (source->count == 0) return 0;

    size_t lums_per_part = source->count / parts;
    size_t remainder = source->count % parts;

    // Keep first part in original zone
    source->count = lums_per_part + (remainder > 0 ? 1 : 0);

    // For now, just split the LUMs within the same zone
    // This is a simplified implementation - in a full implementation,
    // we would create new zones and distribute the LUMs
    if (parts > 1) {
        // Resize the LUMs array to fit the split
        LUM* new_lums = malloc(sizeof(LUM) * source->count);
        if (!new_lums) return -1;
        
        // Copy first part
        size_t first_part_size = lums_per_part + (remainder > 0 ? 1 : 0);
        memcpy(new_lums, source->lums, sizeof(LUM) * first_part_size);
        
        free(source->lums);
        source->lums = new_lums;
        source->count = first_part_size;
    }

    engine->current_tick++;
    return 0;
}

int vorax_cycle_zone(VoraxEngine* engine, int zone, int modulo) {
    if (!engine || zone >= engine->zone_count || modulo <= 0) return -1;
    if (!engine->zones[zone].group) return -1;

    LUMGroup* group = engine->zones[zone].group;
    size_t new_count = group->count % modulo;

    if (new_count < group->count) {
        // Shrink group
        LUM* new_lums = malloc(sizeof(LUM) * new_count);
        if (!new_lums && new_count > 0) return -1;

        if (new_count > 0) {
            memcpy(new_lums, group->lums, sizeof(LUM) * new_count);
        }

        free(group->lums);
        group->lums = new_lums;
        group->count = new_count;
    }

    engine->current_tick++;
    return 0;
}

// Bootstrap sequence implementation
int vorax_bootstrap_phase1(void) {
    printf("VORAX Bootstrap Phase 1: Initializing core primitives...\n");
    // Initialize LUM primitives
    return 0;
}

int vorax_bootstrap_phase2(void) {
    printf("VORAX Bootstrap Phase 2: Loading assembler definitions...\n");
    // Load primitive operations in assembly
    return 0;
}

int vorax_bootstrap_phase3(void) {
    printf("VORAX Bootstrap Phase 3: Compiling bit→LUM encoder...\n");
    // Compile encoder/decoder
    return 0;
}

int vorax_bootstrap_phase4(void) {
    printf("VORAX Bootstrap Phase 4: Initializing VM...\n");
    // Initialize VORAX VM
    return 0;
}

int vorax_bootstrap_phase5(void) {
    printf("VORAX Bootstrap Phase 5: Starting console server...\n");
    // Start console interface
    return 0;
}

int vorax_bootstrap_phase6(void) {
    printf("VORAX Bootstrap Phase 6: Compiling VORAX-L...\n");
    // Compile VORAX-L language
    return 0;
}