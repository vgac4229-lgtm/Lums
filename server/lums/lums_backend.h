#ifndef LUMS_BACKEND_H
#define LUMS_BACKEND_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

// ===== ENUMS ET CONSTANTES =====
typedef enum {
    LUMS_SUCCESS = 0,
    LUMS_ERROR = -1,
    LUMS_MEMORY_ERROR = -2,
    LUMS_VALIDATION_ERROR = -3
} LUMSResult;

// ===== STRUCTURES =====
// LUMGroup definition moved to lums.h to avoid conflicts

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

// ===== FONCTIONS PRINCIPALES =====

// Initialisation et nettoyage
int lums_init(void);
void lums_cleanup(void);

// Gestion des groupes LUM
// LUMGroup* create_lum_group_with_count(size_t count); // Commented out as per the spirit of the changes, assuming it's defined elsewhere.
// void free_lum_group(LUMGroup* group); // Commented out as per the spirit of the changes, assuming it's defined elsewhere.
// LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2); // Commented out as per the spirit of the changes, assuming it's defined elsewhere.

// Calculs mathématiques avancés
double lums_sqrt_newton_raphson(double x, double precision);
bool lums_is_prime_miller_rabin(uint64_t n, int k);
uint64_t lums_fibonacci_authentic(int n);

// Fonctions de validation et logging
bool validate_lum_conservation(size_t total_before, size_t total_after, const char* operation_name);
uint64_t get_nanosecond_timestamp(void);
void log_scientific_operation(const char* operation, double input, double result, long duration_ns);

// Fonctions SIMD et optimisation
static bool simd_available = false;
void init_simd_support(void);
void lums_fusion_vectorized(double* lums_a, double* lums_b, double* result, size_t count);

// Backend système
int lums_backend_init(void);
void lums_backend_cleanup(void);
int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result);
int lums_compute_split_real(uint64_t lum_source, uint64_t* result_a, uint64_t* result_b);
int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result);

// Métriques et status
uint64_t lums_backend_get_total_computations(void);
uint64_t lums_backend_get_energy_consumed(void);
const char* lums_backend_get_status(void);
void lums_backend_status_report(void);

// Tests
int lums_backend_comprehensive_test(void);
char* uint64_to_binary_string(uint64_t value);

#endif // LUMS_BACKEND_H