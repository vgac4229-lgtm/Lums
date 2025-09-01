
#ifndef LUMS_BACKEND_H
#define LUMS_BACKEND_H

#include "lums.h"
#include "electromechanical.h"

// Structures backend avancées
typedef struct {
    uint64_t lum_state;
    uint8_t lum_count;
    uint8_t is_presence_dominant;
    uint64_t last_operation_tick;
} LUMRegister;

typedef struct {
    uint64_t data[8];
    uint8_t used_bits;
    uint8_t conservation_checksum;
    uint64_t creation_timestamp;
} LUMMemoryBlock;

typedef struct {
    ElectromechanicalEngine electro_engine;
    VoraxEngine* vorax_engine;
    uint64_t total_computations;
    uint64_t energy_consumed;
    double computation_time_ms;
    char status_message[256];
    LUMRegister registers[16];
    LUMMemoryBlock memory_blocks[64];
    uint8_t active_memory_blocks;
} LUMSBackend;

// Fonctions backend principales
int lums_backend_init(void);
int lums_compute_fusion_real(uint64_t lum1, uint64_t lum2, uint64_t* result);
int lums_compute_split_real(uint64_t source, uint8_t parts, uint64_t results[]);
int lums_compute_cycle_real(uint64_t source, uint8_t modulo, uint64_t* result);
int lums_compute_flow_real(uint64_t source, uint8_t source_bank, uint8_t target_bank, uint64_t* result);

// Mémoire persistante
int lums_store_memory_real(uint64_t lum_data, uint8_t block_id);
int lums_retrieve_memory_real(uint8_t block_id, uint64_t* lum_data);

// Calculs mathématiques avancés
double lums_compute_sqrt_via_lums(double input);
int lums_test_prime_real(uint64_t candidate);
int lums_generate_fibonacci_real(uint8_t n, uint64_t results[]);

// Monitoring et rapports
void lums_backend_status_report(void);
int lums_backend_comprehensive_test(void);

#endif
