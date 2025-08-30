interface CodeGenerationOptions {
  dataType: string;
  outputMode: string;
  includeHeaders?: boolean;
  optimization?: 'size' | 'speed' | 'portable';
}

export class CCodeGenerator {
  private static readonly HEADER_TEMPLATE = `#ifndef LUMS_H
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
typedef struct {
    LUM* lums;
    size_t count;
    GroupType group_type;
    char* id;
    struct LUMGroup** connections;  // Links to other groups
    size_t connection_count;
} LUMGroup;

// Function declarations
LUM* encode_bit_to_lum(uint64_t input, size_t bit_count);
uint64_t decode_lum_to_bit(LUM* lums, size_t count);
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type);
void free_lum_group(LUMGroup* group);

// VORAX Operations
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2);
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count);
LUMGroup* lum_cycle(LUMGroup* source, int modulo);
LUMGroup* lum_flow(LUMGroup* source, const char* target_zone);

#endif // LUMS_H`;

  private static readonly ENCODER_TEMPLATE = `#include "lums.h"
#include <stdio.h>

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
}`;

  private static readonly DECODER_TEMPLATE = `#include "lums.h"
#include <stdio.h>

/**
 * Decode LUMS back to bits
 */
uint64_t decode_lum_to_bit(LUM* lums, size_t count) {
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
char* decode_to_binary_string(LUM* lums, size_t count) {
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
    result[count] = '\\0';
    
    return result;
}

/**
 * Validate LUM structure integrity
 */
int validate_lums(LUM* lums, size_t count) {
    if (!lums) {
        return 0;
    }
    
    for (size_t i = 0; i < count; i++) {
        if (lums[i].presence > 1) {
            return 0; // Invalid presence value
        }
        if (lums[i].structure_type > LUM_CYCLE) {
            return 0; // Invalid structure type
        }
    }
    
    return 1; // Valid
}`;

  private static readonly OPERATIONS_TEMPLATE = `#include "lums.h"
#include <stdio.h>
#include <string.h>

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
    }
    
    return create_lum_group(fused_lums, total_count, GROUP_CLUSTER);
}

/**
 * VORAX Operation: Split (⇅)
 * Distributes LUMs across multiple zones
 */
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count) {
    if (!source || !source->lums || zones <= 0) {
        *result_count = 0;
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
        size_t zone_count = lums_per_zone + (i < remainder ? 1 : 0);
        
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
        // Create minimal LUM group
        result_count = 1;
    }
    
    LUM* cycled_lums = (LUM*)malloc(sizeof(LUM) * result_count);
    if (!cycled_lums) {
        return NULL;
    }
    
    memcpy(cycled_lums, source->lums, sizeof(LUM) * result_count);
    
    // Mark as cycle type
    for (size_t i = 0; i < result_count; i++) {
        cycled_lums[i].structure_type = LUM_CYCLE;
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
    
    LUMGroup* result = create_lum_group(flowed_lums, source->count, source->group_type);
    if (result) {
        // Store target zone information (simplified)
        result->spatial_data = malloc(strlen(target_zone) + 1);
        if (result->spatial_data) {
            strcpy((char*)result->spatial_data, target_zone);
        }
    }
    
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
}`;

  private static readonly UTILS_TEMPLATE = `#include "lums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * Create a new LUM group
 */
LUMGroup* create_lum_group(LUM* lums, size_t count, GroupType type) {
    if (!lums || count == 0) {
        return NULL;
    }
    
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

/**
 * Print LUM group for debugging
 */
void print_lum_group(LUMGroup* group) {
    if (!group) {
        printf("NULL group\\n");
        return;
    }
    
    printf("LUM Group [%s]: %zu LUMs, Type: %d\\n", 
           group->id ? group->id : "unknown", 
           group->count, 
           group->group_type);
    
    for (size_t i = 0; i < group->count && i < 20; i++) { // Limit output
        printf("%c", group->lums[i].presence ? '•' : '○');
    }
    if (group->count > 20) {
        printf("...");
    }
    printf("\\n");
}

/**
 * Compare two LUM groups
 */
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

/**
 * Clone a LUM group
 */
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
}`;

  generateHeader(): string {
    return CCodeGenerator.HEADER_TEMPLATE;
  }

  generateEncoder(options: CodeGenerationOptions): string {
    let code = CCodeGenerator.ENCODER_TEMPLATE;
    
    // Replace data type placeholders
    code = code.replace(/uint64_t/g, options.dataType);
    
    // Add optimization hints
    if (options.optimization === 'speed') {
      code = code.replace('#include "lums.h"', '#include "lums.h"\n// Optimized for speed\n#pragma GCC optimize("O3")');
    } else if (options.optimization === 'size') {
      code = code.replace('#include "lums.h"', '#include "lums.h"\n// Optimized for size\n#pragma GCC optimize("Os")');
    }
    
    return code;
  }

  generateDecoder(options: CodeGenerationOptions): string {
    let code = CCodeGenerator.DECODER_TEMPLATE;
    
    // Replace data type placeholders
    code = code.replace(/uint64_t/g, options.dataType);
    
    return code;
  }

  generateOperations(options: CodeGenerationOptions): string {
    return CCodeGenerator.OPERATIONS_TEMPLATE;
  }

  generateComplete(options: CodeGenerationOptions): string {
    let complete = '';
    
    if (options.includeHeaders !== false) {
      complete += this.generateHeader() + '\n\n';
    }
    
    complete += this.generateEncoder(options) + '\n\n';
    complete += this.generateDecoder(options) + '\n\n';
    complete += this.generateOperations(options) + '\n\n';
    complete += CCodeGenerator.UTILS_TEMPLATE;
    
    return complete;
  }

  generateMakefile(projectName: string = 'lums'): string {
    return `# LUMS Project Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -I.
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = encoder.c decoder.c operations.c vorax.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/${projectName}

# Default target
all: $(TARGET)

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@

# Clean build files
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Test target
test: $(TARGET)
	./$(TARGET) --test

.PHONY: all clean install test
`;
  }
}

export const cCodeGenerator = new CCodeGenerator();

// Helper functions for common use cases
export function generateEncoderCode(dataType: string = 'uint8_t', outputMode: string = 'struct'): string {
  return cCodeGenerator.generateEncoder({ dataType, outputMode });
}

export function generateDecoderCode(dataType: string = 'uint8_t'): string {
  return cCodeGenerator.generateDecoder({ dataType, outputMode: 'struct' });
}

export function generateOperationsCode(): string {
  return cCodeGenerator.generateOperations({ dataType: 'uint8_t', outputMode: 'struct' });
}

export function generateCompleteLibrary(options: Partial<CodeGenerationOptions> = {}): string {
  const defaultOptions: CodeGenerationOptions = {
    dataType: 'uint8_t',
    outputMode: 'struct',
    includeHeaders: true,
    optimization: 'portable',
    ...options,
  };
  
  return cCodeGenerator.generateComplete(defaultOptions);
}
