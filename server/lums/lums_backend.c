
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "lums_backend.h"

// Variables globales
static uint64_t total_computations = 0;
static uint64_t energy_consumed = 0;
static bool backend_initialized = false;

// Initialisation du backend
int lums_init(void) {
    if (backend_initialized) return LUMS_SUCCESS;
    
    // Créer répertoires de logs
    system("mkdir -p logs/scientific_traces");
    
    // Initialiser support SIMD
    init_simd_support();
    
    backend_initialized = true;
    total_computations = 0;
    energy_consumed = 0;
    
    printf("✅ LUMS Backend initialisé avec succès\n");
    return LUMS_SUCCESS;
}

void lums_cleanup(void) {
    backend_initialized = false;
    printf("✅ LUMS Backend nettoyé\n");
}

// Gestion des groupes LUM
LUMGroup* create_lum_group_with_count(size_t count) {
    if (count == 0) return NULL;
    
    LUMGroup* group = malloc(sizeof(LUMGroup));
    if (!group) return NULL;
    
    group->data = malloc(count * sizeof(double));
    if (!group->data) {
        free(group);
        return NULL;
    }
    
    group->count = count;
    group->capacity = count;
    group->creation_timestamp = get_nanosecond_timestamp();
    
    // Initialiser avec des valeurs par défaut
    for (size_t i = 0; i < count; i++) {
        group->data[i] = 1.0; // Chaque LUM = 1 unité d'énergie
    }
    
    return group;
}

void free_lum_group(LUMGroup* group) {
    if (group) {
        free(group->data);
        free(group);
    }
}

LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2) {
    if (!group1 || !group2) return NULL;
    
    size_t total_count = group1->count + group2->count;
    LUMGroup* result = create_lum_group_with_count(total_count);
    if (!result) return NULL;
    
    // Copier données group1
    memcpy(result->data, group1->data, group1->count * sizeof(double));
    
    // Copier données group2
    memcpy(result->data + group1->count, group2->data, group2->count * sizeof(double));
    
    // Validation conservation
    double sum1 = 0.0, sum2 = 0.0, sum_result = 0.0;
    for (size_t i = 0; i < group1->count; i++) sum1 += group1->data[i];
    for (size_t i = 0; i < group2->count; i++) sum2 += group2->data[i];
    for (size_t i = 0; i < result->count; i++) sum_result += result->data[i];
    
    if (fabs(sum_result - (sum1 + sum2)) > 1e-10) {
        printf("❌ ERREUR: Violation conservation dans fusion\n");
        free_lum_group(result);
        return NULL;
    }
    
    total_computations++;
    energy_consumed += total_count;
    
    return result;
}

// Fonctions backend système
int lums_backend_init(void) {
    return lums_init();
}

void lums_backend_cleanup(void) {
    lums_cleanup();
}

int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result) {
    if (!result) return LUMS_ERROR;
    
    *result = lum_a + lum_b; // Conservation simple
    total_computations++;
    energy_consumed += lum_a + lum_b;
    
    return LUMS_SUCCESS;
}

int lums_compute_split_real(uint64_t lum_input, int zones, uint64_t* results) {
    if (!results || zones <= 0) return LUMS_ERROR;
    
    uint64_t per_zone = lum_input / zones;
    uint64_t remainder = lum_input % zones;
    
    for (int i = 0; i < zones; i++) {
        results[i] = per_zone;
        if (i < remainder) results[i]++; // Distribuer le reste
    }
    
    total_computations++;
    energy_consumed += lum_input;
    
    return LUMS_SUCCESS;
}

int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result) {
    if (!result || modulo <= 0) return LUMS_ERROR;
    
    *result = lum_input % modulo;
    total_computations++;
    energy_consumed += lum_input;
    
    return LUMS_SUCCESS;
}

// Métriques et status
uint64_t lums_backend_get_total_computations(void) {
    return total_computations;
}

uint64_t lums_backend_get_energy_consumed(void) {
    return energy_consumed;
}

const char* lums_backend_get_status(void) {
    static char status_buffer[256];
    snprintf(status_buffer, sizeof(status_buffer),
             "LUMS Backend: %s | Computations: %lu | Energy: %lu",
             backend_initialized ? "ACTIVE" : "INACTIVE",
             total_computations, energy_consumed);
    return status_buffer;
}

void log_scientific_operation(const char* operation, double input, double result, long duration_ns) {
    printf("[SCIENTIFIC_LOG] %s: %.6f → %.6f (%ld ns)\n", 
           operation, input, result, duration_ns);
}

void lums_backend_status_report(void) {
    printf("=== RAPPORT STATUS LUMS BACKEND ===\n");
    printf("État: %s\n", backend_initialized ? "✅ ACTIF" : "❌ INACTIF");
    printf("Calculs totaux: %lu\n", total_computations);
    printf("Énergie consommée: %lu LUMs\n", energy_consumed);
    printf("Support SIMD: %s\n", simd_available ? "✅ Activé" : "❌ Désactivé");
    printf("Timestamp: %lu ns\n", get_nanosecond_timestamp());
}

// Tests
int lums_backend_comprehensive_test(void) {
    printf("=== TESTS COMPLETS BACKEND LUMS ===\n");
    
    // Test 1: Initialisation
    if (lums_init() != LUMS_SUCCESS) {
        printf("❌ Test initialisation échoué\n");
        return LUMS_ERROR;
    }
    printf("✅ Test initialisation réussi\n");
    
    // Test 2: Fusion simple
    uint64_t result;
    if (lums_compute_fusion_real(5, 3, &result) != LUMS_SUCCESS || result != 8) {
        printf("❌ Test fusion échoué\n");
        return LUMS_ERROR;
    }
    printf("✅ Test fusion réussi (5+3=%lu)\n", result);
    
    // Test 3: Split
    uint64_t splits[4];
    if (lums_compute_split_real(10, 4, splits) != LUMS_SUCCESS) {
        printf("❌ Test split échoué\n");
        return LUMS_ERROR;
    }
    uint64_t sum_splits = 0;
    for (int i = 0; i < 4; i++) sum_splits += splits[i];
    if (sum_splits != 10) {
        printf("❌ Conservation split violée\n");
        return LUMS_ERROR;
    }
    printf("✅ Test split réussi (10 → 4 zones, conservation OK)\n");
    
    // Test 4: Cycle
    if (lums_compute_cycle_real(17, 5, &result) != LUMS_SUCCESS || result != 2) {
        printf("❌ Test cycle échoué\n");
        return LUMS_ERROR;
    }
    printf("✅ Test cycle réussi (17 mod 5 = %lu)\n", result);
    
    printf("🏆 TOUS LES TESTS BACKEND RÉUSSIS\n");
    return LUMS_SUCCESS;
}
