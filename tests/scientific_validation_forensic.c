
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "../server/lums/lums_backend.h"
#include "../server/lums/electromechanical.h"

// Forensic validation with deep logging
int main(void) {
    printf("🔬 === VALIDATION FORENSIQUE SCIENTIFIQUE SYSTÈME LUMS/VORAX ===\n");
    printf("Timestamp: %s", ctime(&(time_t){time(NULL)}));
    printf("PID: %d\n", getpid());
    printf("Architecture: %s\n", 
           #ifdef __x86_64__
           "x86_64"
           #elif __aarch64__
           "ARM64"
           #else
           "Unknown"
           #endif
    );
    
    // Initialize scientific logging
    if (init_scientific_logging() != 0) {
        printf("❌ Failed to initialize scientific logging\n");
        return 1;
    }
    
    // Initialize LUMS backend
    printf("\n🔧 Initialisation backend LUMS avec logging forensique...\n");
    if (lums_backend_init() != 0) {
        printf("❌ Failed to initialize LUMS backend\n");
        return 1;
    }
    
    printf("✓ Backend LUMS initialisé avec traçabilité forensique\n");
    
    // FORENSIC TEST 1: Fusion avec validation mathématique rigoureuse
    printf("\n=== TEST FORENSIQUE 1: FUSION AVEC VALIDATION MATHÉMATIQUE ===\n");
    uint64_t lum_a = 0b1010;  // 10 decimal
    uint64_t lum_b = 0b1100;  // 12 decimal
    uint64_t result_fusion;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int ret = lums_compute_fusion(lum_a, lum_b, &result_fusion);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long duration_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    
    if (ret == 0) {
        // Verify conservation: popcount(a) + popcount(b) == popcount(result)
        int bits_a = __builtin_popcountll(lum_a);
        int bits_b = __builtin_popcountll(lum_b);
        int bits_result = __builtin_popcountll(result_fusion);
        
        bool conservation_ok = (bits_a + bits_b) == bits_result;
        
        printf("✓ Fusion: %d+%d LUMs → %d LUMs (conservation: %s)\n", 
               bits_a, bits_b, bits_result, conservation_ok ? "OK" : "ERREUR");
        printf("✓ Temps d'exécution: %ld ns\n", duration_ns);
        
        if (!conservation_ok) {
            printf("❌ ERREUR CRITIQUE: Conservation LUM violée!\n");
            return 1;
        }
        
        log_scientific_operation_detailed("FUSION_FORENSIC", (double)lum_a, (double)result_fusion, duration_ns);
    } else {
        printf("❌ Échec fusion\n");
        return 1;
    }
    
    // FORENSIC TEST 2: Split avec validation inverse
    printf("\n=== TEST FORENSIQUE 2: SPLIT AVEC VALIDATION INVERSE ===\n");
    uint64_t lum_source = result_fusion;  // Use fusion result as source
    uint64_t result_a, result_b;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    ret = lums_compute_split(lum_source, &result_a, &result_b);
    clock_gettime(CLOCK_MONOTONIC, &end);
    duration_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    
    if (ret == 0) {
        int bits_source = __builtin_popcountll(lum_source);
        int bits_a = __builtin_popcountll(result_a);
        int bits_b = __builtin_popcountll(result_b);
        
        bool conservation_ok = bits_source == (bits_a + bits_b);
        
        printf("✓ Split: %d LUMs → %d+%d LUMs (conservation: %s)\n", 
               bits_source, bits_a, bits_b, conservation_ok ? "OK" : "ERREUR");
        printf("✓ Temps d'exécution: %ld ns\n", duration_ns);
        
        if (!conservation_ok) {
            printf("❌ ERREUR CRITIQUE: Conservation LUM violée lors du split!\n");
            return 1;
        }
        
        log_scientific_operation_detailed("SPLIT_FORENSIC", (double)lum_source, (double)(result_a ^ result_b), duration_ns);
    } else {
        printf("❌ Échec split\n");
        return 1;
    }
    
    // FORENSIC TEST 3: Newton-Raphson sqrt avec validation précision
    printf("\n=== TEST FORENSIQUE 3: NEWTON-RAPHSON SQRT AVEC VALIDATION PRÉCISION ===\n");
    double test_values[] = {1.0, 4.0, 9.0, 16.0, 25.0, 64.0, 100.0, 144.0, 256.0, 1024.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        double input = test_values[i];
        double expected = sqrt(input);  // Standard library reference
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        double computed = lums_compute_sqrt(input);
        clock_gettime(CLOCK_MONOTONIC, &end);
        duration_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
        
        double error = fabs(computed - expected);
        double relative_error = error / expected;
        
        printf("✓ √%.1f: computed=%.12f, expected=%.12f, error=%.2e, time=%ld ns\n", 
               input, computed, expected, error, duration_ns);
        
        // Tolerance: 1e-10 for scientific precision
        if (relative_error > 1e-10) {
            printf("❌ ERREUR CRITIQUE: Précision insuffisante pour √%.1f\n", input);
            return 1;
        }
        
        log_scientific_operation_detailed("SQRT_NEWTON_RAPHSON", input, computed, duration_ns);
    }
    
    // FORENSIC TEST 4: Miller-Rabin primalité avec validation exhaustive
    printf("\n=== TEST FORENSIQUE 4: MILLER-RABIN PRIMALITÉ AVEC VALIDATION EXHAUSTIVE ===\n");
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
    uint64_t composites[] = {4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32, 33, 34, 35, 36, 38};
    
    int prime_count = sizeof(primes) / sizeof(primes[0]);
    int composite_count = sizeof(composites) / sizeof(composites[0]);
    
    // Test known primes
    for (int i = 0; i < prime_count; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bool is_prime = lums_test_prime(primes[i]);
        clock_gettime(CLOCK_MONOTONIC, &end);
        duration_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
        
        if (!is_prime) {
            printf("❌ ERREUR CRITIQUE: %lu identifié comme composé (devrait être premier)\n", primes[i]);
            return 1;
        }
        
        printf("✓ %lu confirmé premier (temps: %ld ns)\n", primes[i], duration_ns);
        log_scientific_operation_detailed("PRIME_TEST_TRUE", (double)primes[i], 1.0, duration_ns);
    }
    
    // Test known composites
    for (int i = 0; i < composite_count; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bool is_prime = lums_test_prime(composites[i]);
        clock_gettime(CLOCK_MONOTONIC, &end);
        duration_ns = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
        
        if (is_prime) {
            printf("❌ ERREUR CRITIQUE: %lu identifié comme premier (devrait être composé)\n", composites[i]);
            return 1;
        }
        
        printf("✓ %lu confirmé composé (temps: %ld ns)\n", composites[i], duration_ns);
        log_scientific_operation_detailed("PRIME_TEST_FALSE", (double)composites[i], 0.0, duration_ns);
    }
    
    // FORENSIC TEST 5: Stress test mémoire avec validation checksums
    printf("\n=== TEST FORENSIQUE 5: STRESS TEST MÉMOIRE AVEC CHECKSUMS ===\n");
    char keys[100][32];
    uint64_t values[100];
    
    // Store 100 random values
    for (int i = 0; i < 100; i++) {
        snprintf(keys[i], sizeof(keys[i]), "stress_test_%d", i);
        values[i] = ((uint64_t)rand() << 32) | rand();
        
        if (lums_store_memory(keys[i], values[i]) != 0) {
            printf("❌ ERREUR: Échec stockage clé %s\n", keys[i]);
            return 1;
        }
    }
    
    // Retrieve and validate all values
    for (int i = 0; i < 100; i++) {
        uint64_t retrieved;
        if (lums_retrieve_memory(keys[i], &retrieved) != 0) {
            printf("❌ ERREUR: Échec récupération clé %s\n", keys[i]);
            return 1;
        }
        
        if (retrieved != values[i]) {
            printf("❌ ERREUR CRITIQUE: Corruption mémoire détectée!\n");
            printf("   Clé: %s, Attendu: 0x%lX, Récupéré: 0x%lX\n", keys[i], values[i], retrieved);
            return 1;
        }
    }
    
    printf("✓ Stress test mémoire: 100/100 valeurs validées\n");
    
    // Generate comprehensive forensic report
    printf("\n=== RAPPORT FORENSIQUE FINAL ===\n");
    printf("✅ FUSION: Conservation mathématique validée\n");
    printf("✅ SPLIT: Conservation inverse validée\n");
    printf("✅ SQRT: %d tests précision passés\n", num_tests);
    printf("✅ PRIMALITÉ: %d primes + %d composés validés\n", prime_count, composite_count);
    printf("✅ MÉMOIRE: 100 opérations stress test passées\n");
    
    printf("\n📊 MÉTRIQUES AUTHENTICITY:\n");
    printf("- Total opérations: %d\n", num_tests + prime_count + composite_count + 102);
    printf("- Temps total mesuré: Nanoseconde precision avec clock_gettime\n");
    printf("- Conservation: 100%% respectée (vérification bit-level)\n");
    printf("- Précision: 1e-10 tolerance mathématique\n");
    printf("- Intégrité: 100%% checksums mémoire validés\n");
    
    // Cleanup
    close_scientific_logging();
    lums_backend_cleanup();
    
    printf("\n🏆 === VALIDATION FORENSIQUE COMPLÈTE RÉUSSIE ===\n");
    printf("Le système LUMS/VORAX passe TOUS les tests d'authenticité scientifique.\n");
    
    return 0;
}
