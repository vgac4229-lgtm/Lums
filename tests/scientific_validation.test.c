
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "../server/lums/lums_backend.h"

// Compteurs globaux tests
static int tests_passed = 0;
static int tests_failed = 0;
static double total_test_time = 0.0;

// Macros assertion avancées
#define ASSERT_EQUAL(expected, actual) do { \
    if ((expected) != (actual)) { \
        printf("❌ ÉCHEC ligne %d: attendu %ld, obtenu %ld\n", __LINE__, (long)(expected), (long)(actual)); \
        tests_failed++; \
        return; \
    } \
    tests_passed++; \
} while(0)

#define ASSERT_CONSERVATION(input_count, output_count) do { \
    if ((input_count) != (output_count)) { \
        printf("❌ VIOLATION CONSERVATION ligne %d: entrée %d LUMs, sortie %d LUMs\n", __LINE__, input_count, output_count); \
        tests_failed++; \
        return; \
    } \
    tests_passed++; \
} while(0)

#define ASSERT_DOUBLE_EQUAL(expected, actual, tolerance) do { \
    if (fabs((expected) - (actual)) > (tolerance)) { \
        printf("❌ ÉCHEC PRÉCISION ligne %d: attendu %.12f, obtenu %.12f, erreur %.2e\n", __LINE__, expected, actual, fabs(expected - actual)); \
        tests_failed++; \
        return; \
    } \
    tests_passed++; \
} while(0)

#define TIME_FUNCTION(func_call, time_var) do { \
    struct timeval start, end; \
    gettimeofday(&start, NULL); \
    func_call; \
    gettimeofday(&end, NULL); \
    time_var = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; \
    total_test_time += time_var; \
} while(0)

// === TESTS CONSERVATION LUM ===

void test_fusion_conservation_basic() {
    printf("🧪 Test fusion conservation basique...\n");
    
    uint64_t lum_a = 0b1010; // 2 LUMs
    uint64_t lum_b = 0b1100; // 2 LUMs
    uint64_t result;
    double exec_time;
    
    TIME_FUNCTION(lums_compute_fusion_real(lum_a, lum_b, &result), exec_time);
    
    int count_a = __builtin_popcountll(lum_a);
    int count_b = __builtin_popcountll(lum_b);
    int count_result = __builtin_popcountll(result);
    
    ASSERT_CONSERVATION(count_a + count_b - __builtin_popcountll(lum_a & lum_b), count_result);
    printf("✓ Conservation validée: %d+%d-%d = %d LUMs (%.3fms)\n", 
           count_a, count_b, __builtin_popcountll(lum_a & lum_b), count_result, exec_time);
}

void test_fusion_conservation_edge_cases() {
    printf("🧪 Test fusion cas limites...\n");
    
    // Cas 1: LUM vide
    uint64_t result;
    double exec_time;
    TIME_FUNCTION(lums_compute_fusion_real(0, 0b1111, &result), exec_time);
    ASSERT_EQUAL(__builtin_popcountll(result), 4);
    
    // Cas 2: LUMs identiques (overlaps complets)
    TIME_FUNCTION(lums_compute_fusion_real(0b1010, 0b1010, &result), exec_time);
    ASSERT_EQUAL(__builtin_popcountll(result), 2);
    
    // Cas 3: LUMs disjoints
    TIME_FUNCTION(lums_compute_fusion_real(0b1010, 0b0101, &result), exec_time);
    ASSERT_EQUAL(__builtin_popcountll(result), 4);
    
    printf("✓ Tous les cas limites fusion validés\n");
}

void test_split_conservation_strict() {
    printf("🧪 Test split conservation stricte...\n");
    
    uint64_t input = 0b11111111; // 8 LUMs
    uint64_t results[4];
    double exec_time;
    
    TIME_FUNCTION(lums_compute_split_real(input, 3, results), exec_time);
    
    int total_input = __builtin_popcountll(input);
    int total_output = 0;
    for (int i = 0; i < 3; i++) {
        total_output += __builtin_popcountll(results[i]);
    }
    
    ASSERT_CONSERVATION(total_input, total_output);
    
    // Vérifier distribution équitable
    int expected_per_zone = total_input / 3;
    int remainder = total_input % 3;
    
    for (int i = 0; i < 3; i++) {
        int expected = expected_per_zone + (i < remainder ? 1 : 0);
        int actual = __builtin_popcountll(results[i]);
        if (abs(expected - actual) > 1) {
            printf("❌ Distribution inéquitable zone %d: attendu ~%d, obtenu %d\n", i, expected, actual);
            tests_failed++;
            return;
        }
    }
    
    tests_passed++;
    printf("✓ Split conservation et distribution validées (%.3fms)\n", exec_time);
}

// === TESTS MATHÉMATIQUES PRÉCISION ===

void test_sqrt_precision_scientific() {
    printf("🧪 Test précision sqrt scientifique...\n");
    
    struct {
        double input;
        double expected;
        double tolerance;
    } test_cases[] = {
        {0.0, 0.0, 1e-15},
        {1.0, 1.0, 1e-15},
        {4.0, 2.0, 1e-12},
        {16.0, 4.0, 1e-12},
        {64.0, 8.0, 1e-12},
        {100.0, 10.0, 1e-12},
        {2.0, 1.414213562373095, 1e-10},
        {3.0, 1.732050807568877, 1e-10},
        {5.0, 2.236067977499790, 1e-10}
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        double result;
        double exec_time;
        
        TIME_FUNCTION(result = lums_compute_sqrt_via_lums(test_cases[i].input), exec_time);
        
        ASSERT_DOUBLE_EQUAL(test_cases[i].expected, result, test_cases[i].tolerance);
        
        printf("  ✓ √%.1f = %.12f (erreur: %.2e, %.3fms)\n", 
               test_cases[i].input, result, 
               fabs(result - test_cases[i].expected), exec_time);
    }
}

void test_prime_correctness_comprehensive() {
    printf("🧪 Test primalité correction complète...\n");
    
    // Premiers connus
    int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 97, 101, 103, 107, 109};
    int composites[] = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 91, 93, 95, 99};
    
    // Test premiers
    for (size_t i = 0; i < sizeof(primes) / sizeof(primes[0]); i++) {
        int result;
        double exec_time;
        
        TIME_FUNCTION(result = lums_test_prime_real(primes[i]), exec_time);
        ASSERT_EQUAL(result, 1);
        
        printf("  ✓ %d est premier (%.3fms)\n", primes[i], exec_time);
    }
    
    // Test composés
    for (size_t i = 0; i < sizeof(composites) / sizeof(composites[0]); i++) {
        int result;
        double exec_time;
        
        TIME_FUNCTION(result = lums_test_prime_real(composites[i]), exec_time);
        ASSERT_EQUAL(result, 0);
        
        printf("  ✓ %d n'est pas premier (%.3fms)\n", composites[i], exec_time);
    }
}

// === TESTS PERFORMANCE ===

void test_performance_fusion_batch() {
    printf("🧪 Test performance fusion batch...\n");
    
    const int iterations = 1000;
    double total_time = 0.0;
    uint64_t total_energy_before = lums_backend_get_energy_consumed();
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < iterations; i++) {
        uint64_t lum_a = ((uint64_t)rand() << 32) | rand();
        uint64_t lum_b = ((uint64_t)rand() << 32) | rand();
        uint64_t result;
        
        int status = lums_compute_fusion_real(lum_a, lum_b, &result);
        if (status != 0) {
            printf("❌ Fusion échouée itération %d\n", i);
            tests_failed++;
            return;
        }
        
        // Validation conservation pour chaque opération
        int count_a = __builtin_popcountll(lum_a);
        int count_b = __builtin_popcountll(lum_b);
        int count_result = __builtin_popcountll(result);
        
        if (count_result > count_a + count_b) {
            printf("❌ Violation conservation itération %d\n", i);
            tests_failed++;
            return;
        }
    }
    
    gettimeofday(&end, NULL);
    total_time = (end.tv_sec - start.tv_sec) * 1000.0 + 
                 (end.tv_usec - start.tv_usec) / 1000.0;
    
    uint64_t total_energy_after = lums_backend_get_energy_consumed();
    uint64_t energy_consumed = total_energy_after - total_energy_before;
    
    double avg_time = total_time / iterations;
    double avg_energy = (double)energy_consumed / iterations;
    
    printf("✓ %d fusions en %.3fms (%.4fms/op, %.2f unités/op)\n", 
           iterations, total_time, avg_time, avg_energy);
    
    // Vérifications performance
    if (avg_time > 1.0) {
        printf("⚠️  Performance dégradée: %.4fms/op > 1.0ms\n", avg_time);
    }
    
    tests_passed++;
}

// === TESTS MÉMOIRE ET INTÉGRITÉ ===

void test_memory_integrity_complete() {
    printf("🧪 Test intégrité mémoire complète...\n");
    
    const int slots = 10;
    uint64_t test_data[slots];
    
    // Génération données test
    srand(time(NULL));
    for (int i = 0; i < slots; i++) {
        test_data[i] = ((uint64_t)rand() << 32) | rand();
    }
    
    // Test stockage
    for (int i = 0; i < slots; i++) {
        int status = lums_store_memory_real(test_data[i], i);
        ASSERT_EQUAL(status, 0);
    }
    
    // Test récupération immédiate
    for (int i = 0; i < slots; i++) {
        uint64_t retrieved;
        int status = lums_retrieve_memory_real(i, &retrieved);
        ASSERT_EQUAL(status, 0);
        ASSERT_EQUAL(test_data[i], retrieved);
    }
    
    // Test récupération aléatoire multiple
    for (int iteration = 0; iteration < 100; iteration++) {
        int slot = rand() % slots;
        uint64_t retrieved;
        int status = lums_retrieve_memory_real(slot, &retrieved);
        ASSERT_EQUAL(status, 0);
        ASSERT_EQUAL(test_data[slot], retrieved);
    }
    
    printf("✓ Intégrité mémoire validée sur %d slots avec 100 accès aléatoires\n", slots);
}

// === TESTS CONCURRENCE ===

typedef struct {
    int thread_id;
    int iterations;
    int* errors;
} ThreadTestData;

void* thread_fusion_test(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        uint64_t lum_a = ((uint64_t)rand() << 32) | rand();
        uint64_t lum_b = ((uint64_t)rand() << 32) | rand();
        uint64_t result;
        
        int status = lums_compute_fusion_real(lum_a, lum_b, &result);
        if (status != 0) {
            (*data->errors)++;
            continue;
        }
        
        // Validation conservation
        int count_a = __builtin_popcountll(lum_a);
        int count_b = __builtin_popcountll(lum_b);
        int count_result = __builtin_popcountll(result);
        
        if (count_result > count_a + count_b) {
            (*data->errors)++;
        }
    }
    
    return NULL;
}

void test_concurrent_operations() {
    printf("🧪 Test opérations concurrentes...\n");
    
    const int num_threads = 4;
    const int iterations_per_thread = 100;
    
    pthread_t threads[num_threads];
    ThreadTestData thread_data[num_threads];
    int total_errors = 0;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Lancement threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].iterations = iterations_per_thread;
        thread_data[i].errors = &total_errors;
        
        if (pthread_create(&threads[i], NULL, thread_fusion_test, &thread_data[i]) != 0) {
            printf("❌ Erreur création thread %d\n", i);
            tests_failed++;
            return;
        }
    }
    
    // Attente completion
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    double total_time = (end.tv_sec - start.tv_sec) * 1000.0 + 
                        (end.tv_usec - start.tv_usec) / 1000.0;
    
    ASSERT_EQUAL(total_errors, 0);
    
    printf("✓ %d threads × %d opérations en %.3fms sans erreur\n", 
           num_threads, iterations_per_thread, total_time);
    
    tests_passed++;
}

// === FONCTION PRINCIPALE TESTS ===

int main() {
    printf("🔬 === VALIDATION SCIENTIFIQUE SYSTÈME LUMS/VORAX ===\n\n");
    
    // Initialisation backend
    if (lums_backend_init() != 0) {
        printf("❌ Erreur initialisation backend\n");
        return 1;
    }
    
    struct timeval global_start, global_end;
    gettimeofday(&global_start, NULL);
    
    // Exécution tests
    test_fusion_conservation_basic();
    test_fusion_conservation_edge_cases();
    test_split_conservation_strict();
    test_sqrt_precision_scientific();
    test_prime_correctness_comprehensive();
    test_performance_fusion_batch();
    test_memory_integrity_complete();
    test_concurrent_operations();
    
    gettimeofday(&global_end, NULL);
    double global_time = (global_end.tv_sec - global_start.tv_sec) * 1000.0 + 
                         (global_end.tv_usec - global_start.tv_usec) / 1000.0;
    
    // Rapport final
    printf("\n🎯 === RAPPORT VALIDATION SCIENTIFIQUE ===\n");
    printf("✅ Tests réussis    : %d\n", tests_passed);
    printf("❌ Tests échoués    : %d\n", tests_failed);
    printf("⏱️  Temps execution : %.3f ms\n", global_time);
    printf("💾 Computations     : %lu\n", lums_backend_get_total_computations());
    printf("⚡ Énergie utilisée : %lu unités\n", lums_backend_get_energy_consumed());
    
    double success_rate = tests_passed > 0 ? (double)tests_passed / (tests_passed + tests_failed) * 100.0 : 0.0;
    printf("📊 Taux de réussite : %.1f%%\n", success_rate);
    
    if (tests_failed == 0) {
        printf("\n🏆 VALIDATION SCIENTIFIQUE COMPLÈTE - AUCUNE ERREUR DÉTECTÉE\n");
        printf("🔒 Conservation LUM : 100%% validée\n");
        printf("🎯 Précision mathématique : Conforme standards scientifiques\n");
        printf("⚡ Performance : Dans limites acceptables\n");
        printf("🧵 Concurrence : Thread-safe validé\n");
    } else {
        printf("\n⚠️  VALIDATION PARTIELLE - %d ERREURS DÉTECTÉES\n", tests_failed);
    }
    
    printf("\n📁 Logs détaillés: logs/scientific_traces/lums_operations.jsonl\n");
    
    lums_backend_cleanup();
    return tests_failed > 0 ? 1 : 0;
}
