
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include "../server/lums/lums_backend.h"

#define TEST_ITERATIONS 1000
#define EPSILON 1e-10

typedef struct {
    char test_name[64];
    bool passed;
    double execution_time_ms;
    char details[256];
} TestResult;

static TestResult test_results[50];
static int test_count = 0;

// Fonction de logging des résultats scientifiques
static void log_test_result(const char* test_name, bool passed, double time_ms, const char* details) {
    strcpy(test_results[test_count].test_name, test_name);
    test_results[test_count].passed = passed;
    test_results[test_count].execution_time_ms = time_ms;
    strcpy(test_results[test_count].details, details);
    test_count++;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t timestamp_ns = (uint64_t)tv.tv_sec * 1000000000ULL + (uint64_t)tv.tv_usec * 1000ULL;
    
    FILE* log_file = fopen("logs/scientific_traces/test_validation.jsonl", "a");
    if (log_file) {
        fprintf(log_file, "{\"timestamp_ns\":%lu,\"test\":\"%s\",\"passed\":%s,\"time_ms\":%.3f,\"details\":\"%s\"}\n",
                timestamp_ns, test_name, passed ? "true" : "false", time_ms, details);
        fclose(log_file);
    }
}

// Test de performance Newton-Raphson
static void test_newton_raphson_performance() {
    clock_t start = clock();
    
    // Test sur différentes valeurs
    double test_values[] = {4.0, 16.0, 64.0, 100.0, 1024.0, 65536.0};
    double expected[] = {2.0, 4.0, 8.0, 10.0, 32.0, 256.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    bool all_passed = true;
    char details[256] = "";
    
    for (int i = 0; i < num_tests; i++) {
        double result = lums_sqrt_newton_raphson(test_values[i], EPSILON);
        double error = fabs(result - expected[i]);
        
        if (error > EPSILON) {
            all_passed = false;
            snprintf(details, sizeof(details), "Failed for √%.0f: got %.10f, expected %.10f, error: %e", 
                    test_values[i], result, expected[i], error);
            break;
        }
    }
    
    if (all_passed) {
        snprintf(details, sizeof(details), "All %d tests passed with precision < %e", num_tests, EPSILON);
    }
    
    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    log_test_result("NEWTON_RAPHSON_PRECISION", all_passed, time_ms, details);
}

// Test de conservation LUM rigoureux
static void test_lum_conservation_rigorous() {
    clock_t start = clock();
    
    // Initialiser système LUMS
    if (lums_init() != LUMS_SUCCESS) {
        log_test_result("LUM_CONSERVATION", false, 0.0, "Failed to initialize LUMS system");
        return;
    }
    
    bool all_passed = true;
    char details[256] = "";
    int tests_passed = 0;
    int total_tests = 100;
    
    for (int test = 0; test < total_tests; test++) {
        // Créer groupes LUM aléatoires
        size_t count1 = 1 + rand() % 20;
        size_t count2 = 1 + rand() % 20;
        
        LUMGroup* group1 = create_lum_group_with_count(count1);
        LUMGroup* group2 = create_lum_group_with_count(count2);
        
        if (!group1 || !group2) {
            all_passed = false;
            snprintf(details, sizeof(details), "Memory allocation failed at test %d", test);
            break;
        }
        
        // Test fusion conservation
        size_t expected_total = count1 + count2;
        LUMGroup* fused = lum_fusion(group1, group2);
        
        if (!fused || fused->count != expected_total) {
            all_passed = false;
            snprintf(details, sizeof(details), "Fusion conservation failed: %zu + %zu ≠ %zu", 
                    count1, count2, fused ? fused->count : 0);
            break;
        }
        
        tests_passed++;
        
        // Cleanup
        free_lum_group(group1);
        free_lum_group(group2);
        free_lum_group(fused);
    }
    
    if (all_passed) {
        snprintf(details, sizeof(details), "%d/%d conservation tests passed", tests_passed, total_tests);
    }
    
    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    log_test_result("LUM_CONSERVATION", all_passed, time_ms, details);
}

// Test de primalité avec validation externe
static void test_prime_validation() {
    clock_t start = clock();
    
    // Nombres premiers connus pour validation
    uint64_t known_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 
                               53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113};
    uint64_t known_composites[] = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 
                                   26, 27, 28, 30, 32, 33, 34, 35, 36, 38, 39, 40, 42, 44, 45};
    
    int primes_count = sizeof(known_primes) / sizeof(known_primes[0]);
    int composites_count = sizeof(known_composites) / sizeof(known_composites[0]);
    
    bool all_passed = true;
    char details[256] = "";
    int correct_predictions = 0;
    int total_predictions = primes_count + composites_count;
    
    // Test primes
    for (int i = 0; i < primes_count; i++) {
        if (lums_is_prime_miller_rabin(known_primes[i], 10)) {
            correct_predictions++;
        } else {
            all_passed = false;
            snprintf(details, sizeof(details), "False negative: %lu should be prime", known_primes[i]);
            break;
        }
    }
    
    // Test composites
    if (all_passed) {
        for (int i = 0; i < composites_count; i++) {
            if (!lums_is_prime_miller_rabin(known_composites[i], 10)) {
                correct_predictions++;
            } else {
                all_passed = false;
                snprintf(details, sizeof(details), "False positive: %lu should be composite", known_composites[i]);
                break;
            }
        }
    }
    
    if (all_passed) {
        snprintf(details, sizeof(details), "%d/%d predictions correct (100%% accuracy)", 
                correct_predictions, total_predictions);
    }
    
    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    log_test_result("PRIME_MILLER_RABIN", all_passed, time_ms, details);
}

// Test de performance Fibonacci
static void test_fibonacci_authenticity() {
    clock_t start = clock();
    
    // Valeurs Fibonacci connues pour validation
    uint64_t expected_fib[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610};
    int fib_count = sizeof(expected_fib) / sizeof(expected_fib[0]);
    
    bool all_passed = true;
    char details[256] = "";
    int correct_values = 0;
    
    for (int i = 0; i < fib_count; i++) {
        uint64_t result = lums_fibonacci_authentic(i);
        if (result == expected_fib[i]) {
            correct_values++;
        } else {
            all_passed = false;
            snprintf(details, sizeof(details), "F(%d): got %lu, expected %lu", i, result, expected_fib[i]);
            break;
        }
    }
    
    if (all_passed) {
        snprintf(details, sizeof(details), "%d/%d Fibonacci values correct", correct_values, fib_count);
    }
    
    clock_t end = clock();
    double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    log_test_result("FIBONACCI_AUTHENTICITY", all_passed, time_ms, details);
}

// Fonction principale de test
int main() {
    printf("=== TESTS SCIENTIFIQUES LUMS/VORAX ===\n");
    printf("Démarrage validation complète...\n\n");
    
    // Créer répertoire logs si nécessaire
    system("mkdir -p logs/scientific_traces");
    
    // Exécuter tous les tests
    test_newton_raphson_performance();
    test_lum_conservation_rigorous();
    test_prime_validation();
    test_fibonacci_authenticity();
    
    // Générer rapport final
    printf("RÉSULTATS TESTS SCIENTIFIQUES:\n");
    printf("================================\n");
    
    int passed = 0, failed = 0;
    double total_time = 0.0;
    
    for (int i = 0; i < test_count; i++) {
        TestResult* test = &test_results[i];
        printf("%-25s: %s (%.3f ms) - %s\n", 
               test->test_name, 
               test->passed ? "PASS" : "FAIL", 
               test->execution_time_ms,
               test->details);
        
        if (test->passed) passed++;
        else failed++;
        total_time += test->execution_time_ms;
    }
    
    printf("\n");
    printf("RÉSUMÉ: %d PASS, %d FAIL, %.3f ms total\n", passed, failed, total_time);
    printf("STATUT: %s\n", failed == 0 ? "TOUS LES TESTS RÉUSSIS ✅" : "ÉCHECS DÉTECTÉS ❌");
    
    return failed == 0 ? 0 : 1;
}
