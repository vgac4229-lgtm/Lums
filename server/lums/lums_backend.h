
#ifndef LUMS_BACKEND_H
#define LUMS_BACKEND_H

#include <stdint.h>
#include <stddef.h>

// Structure pour registres LUM
typedef struct {
    uint64_t lum_state;
    uint8_t lum_count;
    uint8_t is_presence_dominant;
    uint64_t last_operation_tick;
} LUMRegister;

// Structure pour blocs mémoire
typedef struct {
    uint64_t data[8];  // 8x64 bits = 512 bits par bloc
    uint8_t used_bits;
    uint8_t conservation_checksum;
    uint64_t creation_timestamp;
} LUMMemoryBlock;

// Structure principale backend LUMS
typedef struct LUMSBackend LUMSBackend;

// Fonctions d'initialisation et nettoyage
int lums_backend_init(void);
void lums_backend_cleanup(void);

// Opérations LUMS fondamentales
int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result);
int lums_compute_split_real(uint64_t lum_input, int zones, uint64_t* results);
int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result);

// Gestion mémoire
int lums_store_memory_real(uint64_t lum_data, int slot_id);
int lums_retrieve_memory_real(int slot_id, uint64_t* result);

// Calculs mathématiques avancés
double lums_compute_sqrt_via_lums(double input);
int lums_test_prime_real(int number);
int lums_generate_fibonacci_real(int count, uint64_t* sequence);

// Métriques et status
uint64_t lums_backend_get_total_computations(void);
uint64_t lums_backend_get_energy_consumed(void);
const char* lums_backend_get_status(void);
void lums_backend_status_report(void);

// Tests
int lums_backend_comprehensive_test(void);

#endif // LUMS_BACKEND_H
