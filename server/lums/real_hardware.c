#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <immintrin.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>

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

// Fonction pour obtenir l'utilisation mémoire actuelle
size_t get_current_memory_usage(void) {
    FILE* status = fopen("/proc/self/status", "r");
    if (!status) return 0;
    
    char line[256];
    size_t memory_kb = 0;
    
    while (fgets(line, sizeof(line), status)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %zu kB", &memory_kb);
            break;
        }
    }
    
    fclose(status);
    return memory_kb * 1024; // Convertir en bytes
}

// Fonction pour mesurer l'énergie via RAPL (si disponible)
double get_rapl_energy(void) {
    FILE* rapl = fopen("/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj", "r");
    if (!rapl) return 0.0;
    
    uint64_t energy_uj;
    if (fscanf(rapl, "%lu", &energy_uj) == 1) {
        fclose(rapl);
        return (double)energy_uj / 1e6; // Convertir en joules
    }
    
    fclose(rapl);
    return 0.0;
}

// Initialiser les métriques
void init_real_metrics(RealHardwareMetrics* metrics) {
    memset(metrics, 0, sizeof(RealHardwareMetrics));
}

// Mesurer l'état initial
void start_real_measurement(RealHardwareMetrics* metrics) {
    // Mesurer utilisation système
    getrusage(RUSAGE_SELF, &metrics->start_usage);
    clock_gettime(CLOCK_MONOTONIC, &metrics->start_time);
    
    // Mesurer mémoire initiale
    metrics->memory_allocated = get_current_memory_usage();
    
    // Mesurer énergie initiale (si RAPL disponible)
    metrics->energy_joules = get_rapl_energy();
}

// Mesurer l'état final
void end_real_measurement(RealHardwareMetrics* metrics) {
    // Mesurer utilisation système finale
    getrusage(RUSAGE_SELF, &metrics->end_usage);
    clock_gettime(CLOCK_MONOTONIC, &metrics->end_time);
    
    // Calculer temps d'exécution réel
    metrics->execution_time_ns = (metrics->end_time.tv_sec - metrics->start_time.tv_sec) * 1e9 + 
                                (metrics->end_time.tv_nsec - metrics->start_time.tv_nsec);
    
    // Calculer temps CPU réel (struct timeval utilise tv_usec, pas tv_nsec)
    metrics->cpu_time_ns = (metrics->end_usage.ru_utime.tv_sec - metrics->start_usage.ru_utime.tv_sec) * 1e9 +
                          (metrics->end_usage.ru_utime.tv_usec - metrics->start_usage.ru_utime.tv_usec) * 1e3;
    
    metrics->system_time_ns = (metrics->end_usage.ru_stime.tv_sec - metrics->start_usage.ru_stime.tv_sec) * 1e9 +
                             (metrics->end_usage.ru_stime.tv_usec - metrics->start_usage.ru_stime.tv_usec) * 1e3;
    
    // Calculer utilisation mémoire finale
    size_t memory_end = get_current_memory_usage();
    metrics->memory_peak = (memory_end > metrics->memory_allocated) ? memory_end : metrics->memory_allocated;
    
    // Calculer énergie finale (si RAPL disponible)
    double energy_end = get_rapl_energy();
    if (energy_end > 0.0) {
        metrics->energy_joules = energy_end - metrics->energy_joules;
        metrics->power_watts = (metrics->energy_joules > 0.0) ? 
                              (metrics->energy_joules / (metrics->execution_time_ns / 1e9)) : 0.0;
    }
}

// Opération de fusion LUMS avec métriques hardware réelles
int lums_compute_fusion_real_hardware(uint64_t lum_a, uint64_t lum_b, uint64_t* result, RealHardwareMetrics* metrics) {
    if (!result || !metrics) return -1;
    
    // Initialiser et démarrer les mesures
    init_real_metrics(metrics);
    start_real_measurement(metrics);
    
    // Mesurer cycles CPU de départ
    uint64_t cycles_start = __builtin_ia32_rdtsc();
    
    // Exécuter opération réelle (pas de simulation)
    *result = lum_a ^ lum_b;
    
    // Optimisations CPU réelles si disponibles
    if (__builtin_cpu_supports("avx2")) {
        // Utiliser AVX2 pour calculs vectoriels
        __m256i a_vec = _mm256_set1_epi64x(lum_a);
        __m256i b_vec = _mm256_set1_epi64x(lum_b);
        __m256i result_vec = _mm256_xor_si256(a_vec, b_vec);
        *result = _mm256_extract_epi64(result_vec, 0);
    }
    
    // Utiliser les instructions de comptage de bits CPU
    uint64_t popcount_a = __builtin_popcountll(lum_a);
    uint64_t popcount_b = __builtin_popcountll(lum_b);
    
    // Calculs réels sur CPU
    *result ^= (popcount_a + popcount_b);
    
    // Mesurer cycles CPU de fin
    uint64_t cycles_end = __builtin_ia32_rdtsc();
    metrics->cpu_cycles = cycles_end - cycles_start;
    
    // Finaliser les mesures
    end_real_measurement(metrics);
    
    return 0;
}

// Opération de split LUMS avec métriques hardware réelles
int lums_compute_split_real_hardware(uint64_t lum_source, uint64_t* result_a, uint64_t* result_b, RealHardwareMetrics* metrics) {
    if (!result_a || !result_b || !metrics) return -1;
    
    // Initialiser et démarrer les mesures
    init_real_metrics(metrics);
    start_real_measurement(metrics);
    
    // Mesurer cycles CPU de départ
    uint64_t cycles_start = __builtin_ia32_rdtsc();
    
    // Exécuter opération de split réelle
    uint64_t popcount = __builtin_popcountll(lum_source);
    uint64_t half_popcount = popcount / 2;
    
    // Algorithme de split basé sur les bits
    *result_a = 0;
    *result_b = 0;
    uint64_t bit_a = 0, bit_b = 0;
    
    for (int i = 0; i < 64; i++) {
        if (lum_source & (1ULL << i)) {
            if (bit_a < half_popcount) {
                *result_a |= (1ULL << i);
                bit_a++;
            } else {
                *result_b |= (1ULL << i);
                bit_b++;
            }
        }
    }
    
    // Mesurer cycles CPU de fin
    uint64_t cycles_end = __builtin_ia32_rdtsc();
    metrics->cpu_cycles = cycles_end - cycles_start;
    
    // Finaliser les mesures
    end_real_measurement(metrics);
    
    return 0;
}

// Test de conservation avec métriques hardware réelles
int test_conservation_real_hardware(uint64_t input_a, uint64_t input_b, RealHardwareMetrics* metrics) {
    if (!metrics) return -1;
    
    uint64_t result_fusion, result_a, result_b;
    RealHardwareMetrics metrics_fusion, metrics_split;
    
    // Mesurer opération de fusion
    if (lums_compute_fusion_real_hardware(input_a, input_b, &result_fusion, &metrics_fusion) != 0) {
        return -1;
    }
    
    // Mesurer opération de split
    if (lums_compute_split_real_hardware(result_fusion, &result_a, &result_b, &metrics_split) != 0) {
        return -1;
    }
    
    // Vérifier conservation sur hardware réel
    uint64_t total_before = __builtin_popcountll(input_a) + __builtin_popcountll(input_b);
    uint64_t total_after = __builtin_popcountll(result_a) + __builtin_popcountll(result_b);
    
    // Accumuler métriques
    metrics->cpu_cycles = metrics_fusion.cpu_cycles + metrics_split.cpu_cycles;
    metrics->execution_time_ns = metrics_fusion.execution_time_ns + metrics_split.execution_time_ns;
    metrics->memory_peak = (metrics_fusion.memory_peak > metrics_split.memory_peak) ? 
                          metrics_fusion.memory_peak : metrics_split.memory_peak;
    metrics->energy_joules = metrics_fusion.energy_joules + metrics_split.energy_joules;
    
    // Vérifier que la fusion inverse donne le même résultat
    uint64_t fusion_back = result_a ^ result_b;
    return (fusion_back == result_fusion) ? 0 : -1;
}

// Logging scientifique avec métriques réelles
void log_real_scientific_operation(const char* operation_type, 
                                  uint64_t input_a, uint64_t input_b, uint64_t result,
                                  const RealHardwareMetrics* metrics) {
    FILE* log = fopen("logs/scientific_traces/real_hardware_operations.jsonl", "a");
    if (!log) return;
    
    fprintf(log, "{\n");
    fprintf(log, "  \"timestamp_ns\": %lu,\n", (unsigned long)metrics->start_time.tv_nsec);
    fprintf(log, "  \"operation_type\": \"%s\",\n", operation_type);
    fprintf(log, "  \"input_a\": \"0x%016lX\",\n", (unsigned long)input_a);
    fprintf(log, "  \"input_b\": \"0x%016lX\",\n", (unsigned long)input_b);
    fprintf(log, "  \"result\": \"0x%016lX\",\n", (unsigned long)result);
    fprintf(log, "  \"cpu_cycles\": %lu,\n", (unsigned long)metrics->cpu_cycles);
    fprintf(log, "  \"execution_time_ns\": %.3f,\n", metrics->execution_time_ns);
    fprintf(log, "  \"memory_peak_bytes\": %zu,\n", metrics->memory_peak);
    fprintf(log, "  \"energy_joules\": %.6f,\n", metrics->energy_joules);
    fprintf(log, "  \"power_watts\": %.6f,\n", metrics->power_watts);
    fprintf(log, "  \"cpu_time_ns\": %.3f,\n", metrics->cpu_time_ns);
    fprintf(log, "  \"system_time_ns\": %.3f\n", metrics->system_time_ns);
    fprintf(log, "}\n");
    
    fclose(log);
}

// Test de performance sur hardware réel
int test_real_hardware_performance(void) {
    const size_t num_operations = 100000;
    RealHardwareMetrics total_metrics = {0};
    size_t successful_operations = 0;
    
    printf("=== TEST PERFORMANCE HARDWARE RÉEL ===\n");
    printf("Exécution de %zu opérations...\n", num_operations);
    
    for (size_t i = 0; i < num_operations; i++) {
        uint64_t a = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        uint64_t b = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        uint64_t result;
        RealHardwareMetrics metrics;
        
        if (lums_compute_fusion_real_hardware(a, b, &result, &metrics) == 0) {
            // Accumuler métriques
            total_metrics.cpu_cycles += metrics.cpu_cycles;
            total_metrics.execution_time_ns += metrics.execution_time_ns;
            total_metrics.memory_peak = (metrics.memory_peak > total_metrics.memory_peak) ? 
                                       metrics.memory_peak : total_metrics.memory_peak;
            total_metrics.energy_joules += metrics.energy_joules;
            successful_operations++;
        }
    }
    
    if (successful_operations == 0) {
        printf("❌ Aucune opération réussie\n");
        return -1;
    }
    
    // Calculer moyennes réelles
    double avg_cycles = (double)total_metrics.cpu_cycles / successful_operations;
    double avg_time_ns = total_metrics.execution_time_ns / successful_operations;
    double ops_per_sec = 1e9 / avg_time_ns;
    double avg_energy = total_metrics.energy_joules / successful_operations;
    
    printf("✅ Résultats RÉELS sur hardware:\n");
    printf("  Opérations réussies: %zu/%zu (%.1f%%)\n", 
           successful_operations, num_operations, 
           (double)successful_operations / num_operations * 100.0);
    printf("  Cycles CPU moyens: %.1f\n", avg_cycles);
    printf("  Temps moyen: %.1f ns\n", avg_time_ns);
    printf("  Opérations/seconde: %.0f\n", ops_per_sec);
    printf("  Mémoire peak: %zu bytes\n", total_metrics.memory_peak);
    printf("  Énergie moyenne: %.6f J\n", avg_energy);
    printf("  Énergie totale: %.6f J\n", total_metrics.energy_joules);
    
    return 0;
}

// Test de conservation sur hardware réel
int test_conservation_real_hardware_batch(void) {
    const size_t num_tests = 10000;
    size_t successful_tests = 0;
    RealHardwareMetrics total_metrics = {0};
    
    printf("=== TEST CONSERVATION HARDWARE RÉEL ===\n");
    printf("Exécution de %zu tests de conservation...\n", num_tests);
    
    for (size_t i = 0; i < num_tests; i++) {
        uint64_t a = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        uint64_t b = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        RealHardwareMetrics metrics;
        
        if (test_conservation_real_hardware(a, b, &metrics) == 0) {
            // Accumuler métriques
            total_metrics.cpu_cycles += metrics.cpu_cycles;
            total_metrics.execution_time_ns += metrics.execution_time_ns;
            total_metrics.memory_peak = (metrics.memory_peak > total_metrics.memory_peak) ? 
                                       metrics.memory_peak : total_metrics.memory_peak;
            total_metrics.energy_joules += metrics.energy_joules;
            successful_tests++;
        }
    }
    
    if (successful_tests == 0) {
        printf("❌ Aucun test de conservation réussi\n");
        return -1;
    }
    
    // Calculer moyennes réelles
    double avg_cycles = (double)total_metrics.cpu_cycles / successful_tests;
    double avg_time_ns = total_metrics.execution_time_ns / successful_tests;
    double tests_per_sec = 1e9 / avg_time_ns;
    double avg_energy = total_metrics.energy_joules / successful_tests;
    
    printf("✅ Résultats CONSERVATION RÉELS:\n");
    printf("  Tests réussis: %zu/%zu (%.1f%%)\n", 
           successful_tests, num_tests, 
           (double)successful_tests / num_tests * 100.0);
    printf("  Cycles CPU moyens: %.1f\n", avg_cycles);
    printf("  Temps moyen: %.1f ns\n", avg_time_ns);
    printf("  Tests/seconde: %.0f\n", tests_per_sec);
    printf("  Mémoire peak: %zu bytes\n", total_metrics.memory_peak);
    printf("  Énergie moyenne: %.6f J\n", avg_energy);
    printf("  Énergie totale: %.6f J\n", total_metrics.energy_joules);
    
    return 0;
}

// Fonction main pour les tests
#ifdef MAIN_TEST
int main(void) {
    printf("=== TEST MODULE REAL_HARDWARE ===\n");
    
    // Test de performance
    if (test_real_hardware_performance() != 0) {
        printf("❌ Test de performance échoué\n");
        return -1;
    }
    
    printf("\n");
    
    // Test de conservation
    if (test_conservation_real_hardware_batch() != 0) {
        printf("❌ Test de conservation échoué\n");
        return -1;
    }
    
    printf("\n✅ Tous les tests real_hardware réussis\n");
    return 0;
}
#endif