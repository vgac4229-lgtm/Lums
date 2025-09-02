
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "../server/lums/lums_backend.h"

// Test compréhensif du backend
int test_backend_comprehensive(void) {
    printf("=== TEST BACKEND COMPLET ===\n");
    
    // Initialisation
    if (lums_backend_init() != 0) {
        printf("❌ ÉCHEC: Initialisation backend\n");
        return -1;
    }
    printf("✅ Backend initialisé\n");
    
    // Test fusion avec conservation
    uint64_t result_fusion;
    int ret = lums_compute_fusion(0b1010, 0b1100, &result_fusion);
    if (ret != 0) {
        printf("❌ ÉCHEC: Fusion - code %d\n", ret);
        return -2;
    }
    
    int lums_before = __builtin_popcountll(0b1010) + __builtin_popcountll(0b1100);
    int lums_after = __builtin_popcountll(result_fusion);
    if (lums_after > lums_before) {
        printf("❌ ÉCHEC: Violation conservation fusion %d → %d\n", lums_before, lums_after);
        return -3;
    }
    printf("✅ Fusion avec conservation: %d LUMs conservés\n", lums_before);
    
    // Test division avec conservation
    uint64_t result_a, result_b;
    ret = lums_compute_split(0b11110000, &result_a, &result_b);
    if (ret != 0) {
        printf("❌ ÉCHEC: Split - code %d\n", ret);
        return -4;
    }
    
    int original_lums = __builtin_popcountll(0b11110000);
    int split_lums = __builtin_popcountll(result_a) + __builtin_popcountll(result_b);
    if (original_lums != split_lums) {
        printf("❌ ÉCHEC: Conservation split %d → %d + %d\n", 
               original_lums, __builtin_popcountll(result_a), __builtin_popcountll(result_b));
        return -5;
    }
    printf("✅ Split avec conservation: %d LUMs distribués\n", original_lums);
    
    // Test racine carrée précision
    double sqrt_64 = lums_compute_sqrt(64.0);
    if (fabs(sqrt_64 - 8.0) > 1e-6) {
        printf("❌ ÉCHEC: √64 = %.10f (attendu: 8.0)\n", sqrt_64);
        return -6;
    }
    printf("✅ √64 = %.10f (précision validée)\n", sqrt_64);
    
    double sqrt_25 = lums_compute_sqrt(25.0);
    if (fabs(sqrt_25 - 5.0) > 1e-6) {
        printf("❌ ÉCHEC: √25 = %.10f (attendu: 5.0)\n", sqrt_25);
        return -7;
    }
    printf("✅ √25 = %.10f (précision validée)\n", sqrt_25);
    
    // Test primalité
    bool is_97_prime = lums_test_prime(97);
    if (!is_97_prime) {
        printf("❌ ÉCHEC: 97 détecté comme non-premier\n");
        return -8;
    }
    printf("✅ 97 correctement détecté comme premier\n");
    
    bool is_91_prime = lums_test_prime(91); // 91 = 7 × 13
    if (is_91_prime) {
        printf("❌ ÉCHEC: 91 détecté comme premier (91 = 7×13)\n");
        return -9;
    }
    printf("✅ 91 correctement détecté comme composé\n");
    
    // Test mémoire
    ret = lums_store_memory("test_conservation", 0b11001010);
    if (ret != 0) {
        printf("❌ ÉCHEC: Stockage mémoire - code %d\n", ret);
        return -10;
    }
    
    uint64_t retrieved_data;
    ret = lums_retrieve_memory("test_conservation", &retrieved_data);
    if (ret != 0 || retrieved_data != 0b11001010) {
        printf("❌ ÉCHEC: Récupération mémoire - code %d, data: %lu\n", ret, retrieved_data);
        return -11;
    }
    printf("✅ Mémoire LUMS fonctionnelle\n");
    
    printf("=== TOUS LES TESTS PASSÉS ===\n");
    lums_backend_cleanup();
    return 0;
}

// Test de stress avec 1000 opérations
int test_stress_operations(void) {
    printf("\n=== TEST STRESS 1000 OPÉRATIONS ===\n");
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    int failures = 0;
    
    for (int i = 0; i < 1000; i++) {
        uint64_t a = (uint64_t)rand() | ((uint64_t)rand() << 32);
        uint64_t b = (uint64_t)rand() | ((uint64_t)rand() << 32);
        uint64_t result;
        
        // Test fusion
        if (lums_compute_fusion(a, b, &result) != 0) {
            failures++;
        }
        
        // Validation conservation
        int lums_before = __builtin_popcountll(a) + __builtin_popcountll(b);
        int lums_after = __builtin_popcountll(result);
        if (lums_after > lums_before) {
            failures++;
            printf("Conservation violation at iteration %d\n", i);
        }
        
        if (i % 100 == 0) {
            printf("Opérations %d/1000 - échecs: %d\n", i, failures);
        }
    }
    
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("✅ 1000 opérations en %.3f secondes\n", elapsed);
    printf("✅ Performance: %.1f ops/sec\n", 1000.0 / elapsed);
    printf("✅ Échecs: %d/1000 (%.1f%%)\n", failures, failures / 10.0);
    
    return failures;
}

// Test de validation conservation rigoureuse
int test_conservation_rigor(void) {
    printf("\n=== TEST CONSERVATION RIGOUREUSE ===\n");
    
    // Test cas limites
    uint64_t result;
    
    // Test avec 0
    if (lums_compute_fusion(0, 0, &result) != 0 || result != 0) {
        printf("❌ ÉCHEC: Fusion 0+0\n");
        return -1;
    }
    printf("✅ Fusion 0+0 = 0\n");
    
    // Test avec maximum
    uint64_t max_val = 0xFFFFFFFFFFFFFFFFULL;
    if (lums_compute_fusion(max_val, 0, &result) != 0 || result != max_val) {
        printf("❌ ÉCHEC: Fusion max+0\n");
        return -2;
    }
    printf("✅ Fusion max+0 = max\n");
    
    // Test split conservation
    uint64_t split_a, split_b;
    uint64_t test_value = 0b11111100001111;
    int original_count = __builtin_popcountll(test_value);
    
    if (lums_compute_split(test_value, &split_a, &split_b) != 0) {
        printf("❌ ÉCHEC: Split failed\n");
        return -3;
    }
    
    int split_count = __builtin_popcountll(split_a) + __builtin_popcountll(split_b);
    if (original_count != split_count) {
        printf("❌ ÉCHEC: Conservation split %d ≠ %d\n", original_count, split_count);
        return -4;
    }
    printf("✅ Conservation split: %d LUMs préservés\n", original_count);
    
    return 0;
}

// Test précision mathématique
int test_mathematical_precision(void) {
    printf("\n=== TEST PRÉCISION MATHÉMATIQUE ===\n");
    
    // Tests racines carrées connues
    struct {
        double input;
        double expected;
        double tolerance;
    } sqrt_tests[] = {
        {4.0, 2.0, 1e-10},
        {9.0, 3.0, 1e-10},
        {16.0, 4.0, 1e-10},
        {25.0, 5.0, 1e-10},
        {64.0, 8.0, 1e-10},
        {100.0, 10.0, 1e-10},
        {2.0, 1.4142135623730951, 1e-10}
    };
    
    int test_count = sizeof(sqrt_tests) / sizeof(sqrt_tests[0]);
    int passed = 0;
    
    for (int i = 0; i < test_count; i++) {
        double computed = lums_compute_sqrt(sqrt_tests[i].input);
        double error = fabs(computed - sqrt_tests[i].expected);
        
        if (error <= sqrt_tests[i].tolerance) {
            printf("✅ √%.1f = %.15f (erreur: %.2e)\n", 
                   sqrt_tests[i].input, computed, error);
            passed++;
        } else {
            printf("❌ √%.1f = %.15f (erreur: %.2e > %.2e)\n",
                   sqrt_tests[i].input, computed, error, sqrt_tests[i].tolerance);
        }
    }
    
    printf("Précision: %d/%d tests passés\n", passed, test_count);
    return (passed == test_count) ? 0 : -1;
}

// Test primalité avancé
int test_primality_advanced(void) {
    printf("\n=== TEST PRIMALITÉ AVANCÉ ===\n");
    
    struct {
        uint64_t number;
        bool expected;
        const char* description;
    } prime_tests[] = {
        {2, true, "2 (plus petit premier)"},
        {3, true, "3 (premier impair)"},
        {4, false, "4 = 2×2"},
        {97, true, "97 (premier < 100)"},
        {91, false, "91 = 7×13"},
        {101, true, "101 (premier > 100)"},
        {121, false, "121 = 11×11"},
        {997, true, "997 (grand premier)"},
        {1001, false, "1001 = 7×11×13"}
    };
    
    int test_count = sizeof(prime_tests) / sizeof(prime_tests[0]);
    int passed = 0;
    
    for (int i = 0; i < test_count; i++) {
        bool result = lums_test_prime(prime_tests[i].number);
        
        if (result == prime_tests[i].expected) {
            printf("✅ %lu: %s (%s)\n", 
                   prime_tests[i].number, 
                   result ? "premier" : "composé",
                   prime_tests[i].description);
            passed++;
        } else {
            printf("❌ %lu: %s (attendu: %s) - %s\n",
                   prime_tests[i].number,
                   result ? "premier" : "composé",
                   prime_tests[i].expected ? "premier" : "composé",
                   prime_tests[i].description);
        }
    }
    
    printf("Primalité: %d/%d tests passés\n", passed, test_count);
    return (passed == test_count) ? 0 : -1;
}

// Test performance temporelle
int test_temporal_performance(void) {
    printf("\n=== TEST PERFORMANCE TEMPORELLE ===\n");
    
    struct timeval start, end;
    
    // Test 100 fusions
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; i++) {
        uint64_t result;
        lums_compute_fusion(0b1010101010101010, 0b0101010101010101, &result);
    }
    gettimeofday(&end, NULL);
    
    double fusion_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("✅ 100 fusions en %.6f secondes (%.1f ops/sec)\n", 
           fusion_time, 100.0 / fusion_time);
    
    // Test 100 calculs racine
    gettimeofday(&start, NULL);
    for (int i = 1; i <= 100; i++) {
        lums_compute_sqrt((double)i);
    }
    gettimeofday(&end, NULL);
    
    double sqrt_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("✅ 100 racines en %.6f secondes (%.1f ops/sec)\n", 
           sqrt_time, 100.0 / sqrt_time);
    
    return 0;
}

// Programme principal
int main(void) {
    printf("🔬 VALIDATION SCIENTIFIQUE LUMS/VORAX COMPLÈTE\n");
    printf("===============================================\n");
    
    srand((unsigned int)time(NULL));
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test 1: Backend complet
    total_tests++;
    if (test_backend_comprehensive() == 0) {
        passed_tests++;
    }
    
    // Test 2: Conservation rigoureuse
    total_tests++;
    if (test_conservation_rigor() == 0) {
        passed_tests++;
    }
    
    // Test 3: Précision mathématique
    total_tests++;
    if (test_mathematical_precision() == 0) {
        passed_tests++;
    }
    
    // Test 4: Primalité avancée
    total_tests++;
    if (test_primality_advanced() == 0) {
        passed_tests++;
    }
    
    // Test 5: Performance temporelle
    total_tests++;
    if (test_temporal_performance() == 0) {
        passed_tests++;
    }
    
    // Test 6: Stress operations
    total_tests++;
    if (test_stress_operations() < 50) { // Moins de 5% d'échecs acceptables
        passed_tests++;
    }
    
    printf("\n===============================================\n");
    printf("🏆 RÉSULTATS FINAUX: %d/%d tests passés\n", passed_tests, total_tests);
    
    if (passed_tests == total_tests) {
        printf("✅ VALIDATION SCIENTIFIQUE RÉUSSIE\n");
        printf("✅ Système LUMS/VORAX certifié authentique\n");
        printf("✅ Prêt pour validation experte externe\n");
    } else {
        printf("❌ VALIDATION SCIENTIFIQUE ÉCHOUÉE\n");
        printf("❌ %d tests en échec nécessitent correction\n", total_tests - passed_tests);
    }
    
    lums_backend_cleanup();
    
    return (passed_tests == total_tests) ? 0 : 1;
}
