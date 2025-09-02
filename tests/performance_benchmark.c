#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>

// ===== BENCHMARKS PERFORMANCE =====

typedef struct {
    char test_name[64];
    long execution_time_ns;
    double operations_per_second;
    size_t memory_used;
} BenchmarkResult;

static BenchmarkResult results[10];
static int result_count = 0;

void add_benchmark_result(const char* name, long time_ns, double ops_per_sec, size_t memory) {
    if (result_count < 10) {
        strncpy(results[result_count].test_name, name, sizeof(results[result_count].test_name) - 1);
        results[result_count].test_name[sizeof(results[result_count].test_name) - 1] = '\0';
        results[result_count].execution_time_ns = time_ns;
        results[result_count].operations_per_second = ops_per_sec;
        results[result_count].memory_used = memory;
        result_count++;
    }
}

// Benchmark allocation/déallocation massive
int benchmark_memory_operations() {
    printf("   Benchmark allocations mémoire...\n");
    
    struct timespec start, end;
    int allocations = 10000;
    void** pointers = malloc(allocations * sizeof(void*));
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Phase allocation
    for (int i = 0; i < allocations; i++) {
        pointers[i] = malloc(1024);
        if (pointers[i]) {
            memset(pointers[i], i % 256, 1024);
        }
    }
    
    // Phase déallocation
    for (int i = 0; i < allocations; i++) {
        free(pointers[i]);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long duration = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    double ops_per_sec = (allocations * 2.0 * 1000000000.0) / duration;  // *2 pour alloc+free
    
    free(pointers);
    
    printf("      ✅ %d allocations+libérations en %ld ns\n", allocations, duration);
    printf("      Performance: %.0f ops/sec\n", ops_per_sec);
    
    add_benchmark_result("Memory Operations", duration, ops_per_sec, allocations * 1024);
    return 1;
}

// Benchmark calculs mathématiques intensifs
int benchmark_mathematical_operations() {
    printf("   Benchmark calculs mathématiques...\n");
    
    struct timespec start, end;
    int operations = 100000;
    double accumulator = 0.0;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 1; i <= operations; i++) {
        double x = (double)i;
        accumulator += sqrt(x) * log(x) + sin(x * 0.001) * cos(x * 0.002);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long duration = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    double ops_per_sec = (operations * 1000000000.0) / duration;
    
    printf("      ✅ %d calculs complexes en %ld ns\n", operations, duration);
    printf("      Performance: %.0f ops/sec\n", ops_per_sec);
    printf("      Résultat: %.10f\n", accumulator);
    
    add_benchmark_result("Mathematical Operations", duration, ops_per_sec, 0);
    return 1;
}

// Benchmark opérations bit-à-bit intensives
int benchmark_bitwise_operations() {
    printf("   Benchmark opérations bit-à-bit...\n");
    
    struct timespec start, end;
    int operations = 1000000;
    uint64_t accumulator = 0x1234567890ABCDEFULL;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < operations; i++) {
        uint64_t value = (uint64_t)i;
        accumulator ^= value;
        accumulator = (accumulator << 1) | (accumulator >> 63);  // Rotation
        accumulator ^= (accumulator >> 32);
        accumulator *= 0x9E3779B97F4A7C15ULL;  // Multiplication par nombre premier
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long duration = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    double ops_per_sec = (operations * 1000000000.0) / duration;
    
    printf("      ✅ %d opérations bit-à-bit en %ld ns\n", operations, duration);
    printf("      Performance: %.0f ops/sec\n", ops_per_sec);
    printf("      Résultat: 0x%lX\n", accumulator);
    
    add_benchmark_result("Bitwise Operations", duration, ops_per_sec, 0);
    return 1;
}

// Benchmark accès mémoire séquentiel vs aléatoire
int benchmark_memory_access_patterns() {
    printf("   Benchmark patterns d'accès mémoire...\n");
    
    size_t array_size = 1024 * 1024;  // 1M éléments
    uint64_t* array = malloc(array_size * sizeof(uint64_t));
    
    // Initialisation
    for (size_t i = 0; i < array_size; i++) {
        array[i] = i;
    }
    
    struct timespec start, end;
    uint64_t sum = 0;
    
    // Test accès séquentiel
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < array_size; i++) {
        sum += array[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long sequential_time = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    
    // Test accès aléatoire
    srand(12345);  // Seed fixe pour reproductibilité
    sum = 0;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < array_size; i++) {
        size_t index = rand() % array_size;
        sum += array[index];
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long random_time = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    
    double seq_bandwidth = (array_size * sizeof(uint64_t) * 1000000000.0) / sequential_time;
    double rand_bandwidth = (array_size * sizeof(uint64_t) * 1000000000.0) / random_time;
    
    printf("      ✅ Accès séquentiel: %ld ns (%.2f MB/s)\n", sequential_time, seq_bandwidth / (1024*1024));
    printf("      ✅ Accès aléatoire: %ld ns (%.2f MB/s)\n", random_time, rand_bandwidth / (1024*1024));
    printf("      Ratio performance: %.2fx\n", (double)random_time / sequential_time);
    
    free(array);
    
    add_benchmark_result("Sequential Memory Access", sequential_time, seq_bandwidth, array_size * sizeof(uint64_t));
    add_benchmark_result("Random Memory Access", random_time, rand_bandwidth, array_size * sizeof(uint64_t));
    return 1;
}

// Benchmark stress test multi-opérations
int benchmark_stress_test() {
    printf("   Benchmark stress test combiné...\n");
    
    struct timespec start, end;
    int iterations = 1000;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < iterations; i++) {
        // Allocation mémoire
        void* ptr = malloc(1024);
        if (!ptr) continue;
        
        // Opérations mathématiques
        double result = sqrt((double)i) * log((double)i + 1);
        
        // Opérations bit-à-bit
        uint64_t bitwise = (uint64_t)i;
        bitwise ^= bitwise >> 32;
        bitwise *= 0x9E3779B97F4A7C15ULL;
        
        // Écriture mémoire
        memset(ptr, (int)bitwise, 1024);
        
        // Libération
        free(ptr);
        
        // Utilisation des résultats pour éviter l'optimisation
        if (result < 0 || bitwise == 0) break;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long duration = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    double ops_per_sec = (iterations * 1000000000.0) / duration;
    
    printf("      ✅ %d itérations stress en %ld ns\n", iterations, duration);
    printf("      Performance: %.0f itérations/sec\n", ops_per_sec);
    
    add_benchmark_result("Stress Test Combined", duration, ops_per_sec, 0);
    return 1;
}

void print_benchmark_summary() {
    printf("\n=== RÉCAPITULATIF BENCHMARKS ===\n");
    printf("%-25s | %12s | %12s | %10s\n", "Test", "Temps (ms)", "Ops/sec", "Mémoire");
    printf("----------------------------------------------------------------\n");
    
    for (int i = 0; i < result_count; i++) {
        printf("%-25s | %9.3f ms | %9.0f | %7.1f KB\n",
               results[i].test_name,
               results[i].execution_time_ns / 1000000.0,
               results[i].operations_per_second,
               results[i].memory_used / 1024.0);
    }
}

int main() {
    printf("=== BENCHMARKS PERFORMANCE LUMS/VORAX ===\n\n");
    
    int benchmarks_completed = 0;
    
    benchmarks_completed += benchmark_memory_operations();
    benchmarks_completed += benchmark_mathematical_operations();
    benchmarks_completed += benchmark_bitwise_operations();
    benchmarks_completed += benchmark_memory_access_patterns();
    benchmarks_completed += benchmark_stress_test();
    
    print_benchmark_summary();
    
    printf("\n🚀 BENCHMARKS TERMINÉS: %d/5 tests exécutés avec succès\n", benchmarks_completed);
    
    return 0;
}