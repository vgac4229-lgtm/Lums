#include "lums.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create simplified VORAX engine
 */
VoraxEngine* create_vorax_engine(void) {
    VoraxEngine* engine = (VoraxEngine*)malloc(sizeof(VoraxEngine));
    if (!engine) return NULL;
    
    engine->zones = NULL;
    engine->zone_count = 0;
    engine->memory_slots = NULL;
    engine->memory_count = 0;
    engine->last_error = NULL;
    memset(engine->error_message, 0, sizeof(engine->error_message));
    
    // Initialize zone names
    for (int i = 0; i < MAX_ZONES; i++) {
        engine->zone_names[i] = NULL;
    }
    
    engine->current_tick = 0;
    engine->energy_budget = 1000.0;
    
    printf("✓ VORAX Engine créé\n");
    return engine;
}

/**
 * Free VORAX engine
 */
void free_vorax_engine(VoraxEngine* engine) {
    if (!engine) return;
    
    // Free zones
    if (engine->zones) {
        for (size_t i = 0; i < engine->zone_count; i++) {
            if (engine->zones[i].name) {
                free(engine->zones[i].name);
            }
            free_lum_group(engine->zones[i].group);
        }
        free(engine->zones);
    }
    
    // Free memory slots
    if (engine->memory_slots) {
        for (size_t i = 0; i < engine->memory_count; i++) {
            if (engine->memory_slots[i].name) {
                free(engine->memory_slots[i].name);
            }
            free_lum_group(engine->memory_slots[i].stored_group);
        }
        free(engine->memory_slots);
    }
    
    // Free zone names
    for (int i = 0; i < MAX_ZONES; i++) {
        if (engine->zone_names[i]) {
            free(engine->zone_names[i]);
        }
    }
    
    if (engine->last_error) {
        free(engine->last_error);
    }
    
    free(engine);
    printf("✓ VORAX Engine libéré\n");
}

/**
 * Set error message
 */
void vorax_set_error(VoraxEngine* engine, const char* error_msg) {
    if (!engine || !error_msg) return;
    
    strncpy(engine->error_message, error_msg, sizeof(engine->error_message) - 1);
    engine->error_message[sizeof(engine->error_message) - 1] = '\0';
}

/**
 * Get last error
 */
const char* vorax_get_last_error(VoraxEngine* engine) {
    if (!engine) return "Engine is NULL";
    return engine->error_message;
}

/**
 * Clear error
 */
void vorax_clear_error(VoraxEngine* engine) {
    if (!engine) return;
    memset(engine->error_message, 0, sizeof(engine->error_message));
}