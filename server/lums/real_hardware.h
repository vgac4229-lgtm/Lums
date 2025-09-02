#ifndef REAL_HARDWARE_H
#define REAL_HARDWARE_H

#include <stdint.h>
#include <stddef.h>
#include <sys/resource.h>
#include <time.h>

// Métriques hardware réelles
typedef struct {
    // Métriques CPU réelles
    uint64_t cpu_cycles;
    uint64_t instructions;
    uint64_t cache_misses;
    uint64_t branch_misses;
    
    // Métriques RAM réelles
    size_t memory_allocated;
    size_t memory_peak;
    uint64_t memory_accesses;
    
    // Métriques énergie réelles
    double energy_joules;
    double power_watts;
    
    // Métriques temporelles réelles
    double execution_time_ns;
    double cpu_time_ns;
    double system_time_ns;
    
    // Métriques système réelles
    struct rusage start_usage;
    struct rusage end_usage;
    struct timespec start_time;
    struct timespec end_time;
} RealHardwareMetrics;

// Fonctions de mesure hardware réelles
size_t get_current_memory_usage(void);
double get_rapl_energy(void);
void init_real_metrics(RealHardwareMetrics* metrics);
void start_real_measurement(RealHardwareMetrics* metrics);
void end_real_measurement(RealHardwareMetrics* metrics);

// Opérations LUMS avec métriques hardware réelles
int lums_compute_fusion_real_hardware(uint64_t lum_a, uint64_t lum_b, uint64_t* result, RealHardwareMetrics* metrics);
int lums_compute_split_real_hardware(uint64_t lum_source, uint64_t* result_a, uint64_t* result_b, RealHardwareMetrics* metrics);

// Tests de validation sur hardware réel
int test_conservation_real_hardware(uint64_t input_a, uint64_t input_b, RealHardwareMetrics* metrics);
int test_real_hardware_performance(void);
int test_conservation_real_hardware_batch(void);

// Logging scientifique avec métriques réelles
void log_real_scientific_operation(const char* operation_type, 
                                  uint64_t input_a, uint64_t input_b, uint64_t result,
                                  const RealHardwareMetrics* metrics);

#endif // REAL_HARDWARE_H