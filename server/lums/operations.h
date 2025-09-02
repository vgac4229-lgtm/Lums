#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "lums.h"

// Core VORAX operations
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2);
LUMGroup** lum_split(LUMGroup* source, int zones, size_t* result_count);
LUMGroup* lum_cycle(LUMGroup* source, int modulo);
LUMGroup* lum_flow(LUMGroup* source, const char* target_zone);

// Advanced operations
LUMGroup* lum_organic_fusion(LUMGroup** groups, size_t group_count);
LUMGroup** lum_contextual_split(LUMGroup* source, size_t* result_count);

// Memory management
void free_split_result(LUMGroup** groups, size_t count);

#endif // OPERATIONS_H