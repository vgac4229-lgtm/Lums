#ifndef LUMS_H
#define LUMS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// LUMS Structure Types
typedef enum {
    LUM_LINEAR = 0,
    LUM_GROUP = 1,
    LUM_NODE = 2,
    LUM_CYCLE = 3
} LumStructureType;

// Group Types
typedef enum {
    GROUP_LINEAR = 0,
    GROUP_CLUSTER = 1,
    GROUP_NODE = 2,
    GROUP_MEMORY = 3
} GroupType;

// Core LUM structure
typedef struct {
    uint8_t presence;              // 0 or 1
    LumStructureType structure_type;
    void* spatial_data;            // Optional spatial information
    struct {
        int x, y;                  // Position in space
    } position;
} LUM;

// LUM Group structure
typedef struct LUMGroup {
    LUM* lums;
    size_t count;
    GroupType group_type;
    char* id;
    struct LUMGroup** connections;  // Links to other groups
    size_t connection_count;
    void* spatial_data;            // Additional spatial metadata
} LUMGroup;

// VORAX Zone structure
typedef struct {
    char* name;
    LUMGroup* group;
    struct {
        int x, y, width, height;
    } bounds;
} VoraxZone;

// VORAX Memory structure
typedef struct {
    char* name;
    LUMGroup* stored_group;
    time_t timestamp;
} VoraxMemory;

// VORAX Engine state
typedef struct {
    VoraxZone* zones;
    size_t zone_count;
    VoraxMemory* memory_slots;
    size_t memory_count;
    char* last_error;
} VoraxEngine;

// Core encoding/decoding functions
LUM* encode_bit_to_lum(uint64_t input, size_t bit_count);
uint64_t decode_lum_to_bit(LUM* lums, size_t count);
LUMGroup* encode_binary_string(const char* binary_str);
char* decode_to_binary_string(LUM* lums, size_t count);

// Group management
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type);
void free_lum_group(LUMGroup* group);
LUMGroup* clone_lum_group(LUMGroup* source);
int compare_lum_groups(LUMGroup* group1, LUMGroup* group2);

// VORAX Operations
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2);
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count);
LUMGroup* lum_cycle(LUMGroup* source, int modulo);
LUMGroup* lum_flow(LUMGroup* source, const char* target_zone);

// VORAX Engine functions
VoraxEngine* create_vorax_engine(void);
void free_vorax_engine(VoraxEngine* engine);
int vorax_add_zone(VoraxEngine* engine, const char* name, int x, int y, int width, int height);
int vorax_set_zone_group(VoraxEngine* engine, const char* zone_name, LUMGroup* group);
LUMGroup* vorax_get_zone_group(VoraxEngine* engine, const char* zone_name);
int vorax_store_memory(VoraxEngine* engine, const char* name, LUMGroup* group);
LUMGroup* vorax_retrieve_memory(VoraxEngine* engine, const char* name);

// Validation and debugging
int validate_lums(LUM* lums, size_t count);
void print_lum_group(LUMGroup* group);
void print_vorax_engine_state(VoraxEngine* engine);

// Error handling
const char* vorax_get_last_error(VoraxEngine* engine);
void vorax_clear_error(VoraxEngine* engine);

#endif // LUMS_H
