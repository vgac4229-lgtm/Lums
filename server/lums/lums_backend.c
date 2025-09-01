#include "lums.h"
#include "electromechanical.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

// Moteur global LUMS
static LUMSBackend g_backend;
static int g_backend_initialized = 0;

// Structure backend LUMS complète
typedef struct {
    ElectromechanicalEngine electro_engine;
    VoraxEngine* vorax_engine;
    uint64_t total_computations;
    uint64_t energy_consumed;
    double computation_time_ms;
    char status_message[256];
    LUMRegister registers[16];
    LUMMemoryBlock memory_blocks[64];
    uint8_t active_memory_blocks;
} LUMSBackend;

// Registres LUM pour calculs réels
typedef struct {
    uint64_t lum_state;
    uint8_t lum_count;
    uint8_t is_presence_dominant;
    uint64_t last_operation_tick;
} LUMRegister;

// Blocs mémoire LUM
typedef struct {
    uint64_t data[8];  // 8x64 bits = 512 bits par bloc
    uint8_t used_bits;
    uint8_t conservation_checksum;
    uint64_t creation_timestamp;
} LUMMemoryBlock;

// Initialisation backend LUMS complet
int lums_backend_init(void) {
    if (g_backend_initialized) {
        return 0; // Déjà initialisé
    }

    printf("🔧 Initialisation backend LUMS/VORAX...\n");

    // Reset structure
    memset(&g_backend, 0, sizeof(LUMSBackend));

    // Initialisation moteur électromécanique
    if (electromechanical_engine_init(&g_backend.electro_engine) != 0) {
        strcpy(g_backend.status_message, "Échec init moteur électromécanique");
        return -1;
    }

    // Initialisation moteur VORAX
    g_backend.vorax_engine = create_vorax_engine();
    if (!g_backend.vorax_engine) {
        strcpy(g_backend.status_message, "Échec init moteur VORAX");
        return -2;
    }

    // Initialisation registres
    for (int i = 0; i < 16; i++) {
        g_backend.registers[i].lum_state = 0;
        g_backend.registers[i].lum_count = 0;
        g_backend.registers[i].is_presence_dominant = 0;
        g_backend.registers[i].last_operation_tick = 0;
    }

    // Initialisation blocs mémoire
    for (int i = 0; i < 64; i++) {
        memset(g_backend.memory_blocks[i].data, 0, sizeof(g_backend.memory_blocks[i].data));
        g_backend.memory_blocks[i].used_bits = 0;
        g_backend.memory_blocks[i].conservation_checksum = 0;
        g_backend.memory_blocks[i].creation_timestamp = 0;
    }

    g_backend.active_memory_blocks = 0;
    g_backend.total_computations = 0;
    g_backend.energy_consumed = 0;
    g_backend.computation_time_ms = 0.0;
    strcpy(g_backend.status_message, "Backend LUMS initialisé avec succès");

    g_backend_initialized = 1;
    printf("✓ Backend LUMS initialisé\n");
    return 0;
}

// Nettoyage backend
void lums_backend_cleanup(void) {
    if (!g_backend_initialized) return;

    printf("🧹 Nettoyage backend LUMS...\n");

    if (g_backend.vorax_engine) {
        free_vorax_engine(g_backend.vorax_engine);
        g_backend.vorax_engine = NULL;
    }

    electromechanical_engine_cleanup(&g_backend.electro_engine);
    g_backend_initialized = 0;

    printf("✓ Backend LUMS nettoyé\n");
}

// Calcul de fusion réel avec conservation LUM
int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result) {
    if (!g_backend_initialized || !result) return -1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Conservation des LUMs : fusion = OR logique avec validation
    int count_a = __builtin_popcountll(lum_a);
    int count_b = __builtin_popcountll(lum_b);

    *result = lum_a | lum_b;  // Fusion par OR logique
    int count_result = __builtin_popcountll(*result);

    // Validation conservation (peut être <= somme car overlaps)
    if (count_result > count_a + count_b) {
        return -2; // Violation conservation
    }

    // Simulation électromécanique
    electromechanical_operate_relay(&g_backend.electro_engine, 0, 1);
    electromechanical_operate_relay(&g_backend.electro_engine, 1, 1);

    // Mise à jour métriques
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += 15; // 15 unités par fusion
    g_backend.computation_time_ms += time_ms;

    snprintf(g_backend.status_message, sizeof(g_backend.status_message),
             "Fusion: %d+%d LUMs → %d LUMs (%.2fms)",
             count_a, count_b, count_result, time_ms);

    return 0;
}

// Calcul de division réel avec conservation
int lums_compute_split_real(uint64_t lum_input, int zones, uint64_t* results) {
    if (!g_backend_initialized || !results || zones <= 0) return -1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int total_lums = __builtin_popcountll(lum_input);
    int lums_per_zone = total_lums / zones;
    int remainder = total_lums % zones;

    // Distribution équitable des LUMs
    uint64_t mask = 1ULL;
    int current_zone = 0;
    int lums_in_current = 0;

    for (int i = 0; i < zones; i++) {
        results[i] = 0;
    }

    for (int bit = 0; bit < 64 && current_zone < zones; bit++) {
        if (lum_input & (1ULL << bit)) {
            results[current_zone] |= (1ULL << lums_in_current);
            lums_in_current++;

            int target_for_zone = lums_per_zone + (current_zone < remainder ? 1 : 0);
            if (lums_in_current >= target_for_zone) {
                current_zone++;
                lums_in_current = 0;
            }
        }
    }

    // Validation conservation
    int total_output = 0;
    for (int i = 0; i < zones; i++) {
        total_output += __builtin_popcountll(results[i]);
    }

    if (total_output != total_lums) {
        return -2; // Violation conservation
    }

    // Mise à jour métriques
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += 10 * zones;
    g_backend.computation_time_ms += time_ms;

    snprintf(g_backend.status_message, sizeof(g_backend.status_message),
             "Split: %d LUMs → %d zones (%.2fms)",
             total_lums, zones, time_ms);

    return 0;
}

// Calcul de cycle réel
int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result) {
    if (!g_backend_initialized || !result || modulo <= 0) return -1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int input_count = __builtin_popcountll(lum_input);
    int result_count = input_count % modulo;

    if (result_count == 0) {
        *result = 1; // Au moins 1 LUM présent
        result_count = 1;
    } else {
        // Conservation des premiers bits
        *result = lum_input & ((1ULL << result_count) - 1);
    }

    // Simulation électromécanique cyclique
    for (int i = 0; i < modulo; i++) {
        electromechanical_operate_relay(&g_backend.electro_engine, i % 8, i % 2);
    }

    // Mise à jour métriques
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += 5 * modulo;
    g_backend.computation_time_ms += time_ms;

    snprintf(g_backend.status_message, sizeof(g_backend.status_message),
             "Cycle: %d LUMs mod %d → %d LUMs (%.2fms)",
             input_count, modulo, result_count, time_ms);

    return 0;
}

// Stockage mémoire avec checksum
int lums_store_memory_real(uint64_t lum_data, int slot_id) {
    if (!g_backend_initialized || slot_id < 0 || slot_id >= 64) return -1;

    LUMMemoryBlock* block = &g_backend.memory_blocks[slot_id];

    // Stockage données
    block->data[0] = lum_data;
    for (int i = 1; i < 8; i++) {
        block->data[i] = 0;
    }

    block->used_bits = __builtin_popcountll(lum_data);
    block->creation_timestamp = time(NULL);

    // Calcul checksum CRC32 simplifié
    uint32_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= (uint32_t)(block->data[i] & 0xFFFFFFFF);
        checksum ^= (uint32_t)((block->data[i] >> 32) & 0xFFFFFFFF);
    }
    block->conservation_checksum = (uint8_t)(checksum & 0xFF);

    if (slot_id >= g_backend.active_memory_blocks) {
        g_backend.active_memory_blocks = slot_id + 1;
    }

    return 0;
}

// Récupération mémoire avec validation
int lums_retrieve_memory_real(int slot_id, uint64_t* result) {
    if (!g_backend_initialized || !result || slot_id < 0 || slot_id >= 64) return -1;

    LUMMemoryBlock* block = &g_backend.memory_blocks[slot_id];

    if (block->used_bits == 0) {
        return -2; // Slot vide
    }

    // Validation checksum
    uint32_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum ^= (uint32_t)(block->data[i] & 0xFFFFFFFF);
        checksum ^= (uint32_t)((block->data[i] >> 32) & 0xFFFFFFFF);
    }

    if ((uint8_t)(checksum & 0xFF) != block->conservation_checksum) {
        return -3; // Corruption détectée
    }

    *result = block->data[0];
    return 0;
}

// Calcul sqrt via méthode Newton avec LUMs
double lums_compute_sqrt_via_lums(double input) {
    if (!g_backend_initialized || input < 0) return -1.0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (input == 0.0) return 0.0;
    if (input == 1.0) return 1.0;

    // Méthode de Newton: x_{n+1} = (x_n + input/x_n) / 2
    double x = input / 2.0;  // Estimation initiale
    double precision = 1e-10;
    int max_iterations = 50;

    for (int i = 0; i < max_iterations; i++) {
        double x_new = (x + input / x) / 2.0;

        // Simulation LUM pour chaque itération
        uint64_t lum_state = (uint64_t)(x_new * 1000000) & 0xFFFFFFFF;
        electromechanical_operate_relay(&g_backend.electro_engine, 
                                      i % 8, 
                                      __builtin_popcount((uint32_t)lum_state) % 2);

        if (fabs(x_new - x) < precision) {
            x = x_new;
            break;
        }
        x = x_new;
    }

    // Mise à jour métriques
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += 25;
    g_backend.computation_time_ms += time_ms;

    return x;
}

// Test primalité via division LUM
int lums_test_prime_real(int number) {
    if (!g_backend_initialized || number < 2) return 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (number == 2) return 1;
    if (number % 2 == 0) return 0;

    // Test division jusqu'à √number
    int sqrt_n = (int)sqrt(number);
    for (int i = 3; i <= sqrt_n; i += 2) {
        if (number % i == 0) {
            // Simulation LUM pour division
            uint64_t lum_div = ((uint64_t)number << 16) | i;
            electromechanical_operate_relay(&g_backend.electro_engine, 
                                          i % 8, 
                                          __builtin_popcountll(lum_div) % 2);

            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                             (end.tv_nsec - start.tv_nsec) / 1000000.0;

            g_backend.total_computations++;
            g_backend.energy_consumed += 8;
            g_backend.computation_time_ms += time_ms;

            return 0; // Non premier
        }
    }

    // Mise à jour métriques pour nombre premier
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += 12;
    g_backend.computation_time_ms += time_ms;

    return 1; // Premier
}

// Génération Fibonacci avec LUMs
int lums_generate_fibonacci_real(int count, uint64_t* sequence) {
    if (!g_backend_initialized || !sequence || count <= 0) return -1;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (count >= 1) sequence[0] = 0;
    if (count >= 2) sequence[1] = 1;

    for (int i = 2; i < count; i++) {
        sequence[i] = sequence[i-1] + sequence[i-2];

        // Simulation LUM pour chaque nombre Fibonacci
        electromechanical_operate_relay(&g_backend.electro_engine, 
                                      i % 8, 
                                      __builtin_popcountll(sequence[i]) % 2);

        // Protection overflow
        if (sequence[i] < sequence[i-1]) {
            return -2; // Overflow détecté
        }
    }

    // Mise à jour métriques
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                     (end.tv_nsec - start.tv_nsec) / 1000000.0;

    g_backend.total_computations++;
    g_backend.energy_consumed += count * 3;
    g_backend.computation_time_ms += time_ms;

    return 0;
}

// Accesseurs pour métriques
uint64_t lums_backend_get_total_computations(void) {
    return g_backend_initialized ? g_backend.total_computations : 0;
}

uint64_t lums_backend_get_energy_consumed(void) {
    return g_backend_initialized ? g_backend.energy_consumed : 0;
}

const char* lums_backend_get_status(void) {
    return g_backend_initialized ? g_backend.status_message : "Backend non initialisé";
}

// Rapport d'état backend complet
void lums_backend_status_report(void) {
    if (!g_backend_initialized) {
        printf("❌ Backend LUMS non initialisé\n");
        return;
    }

    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    RAPPORT BACKEND LUMS                   ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Computations totales: %8lu                             ║\n", g_backend.total_computations);
    printf("║ Énergie consommée   : %8lu unités                      ║\n", g_backend.energy_consumed);
    printf("║ Temps total         : %8.2f ms                          ║\n", g_backend.computation_time_ms);
    printf("║ Blocs mémoire actifs: %8d/64                           ║\n", g_backend.active_memory_blocks);

    if (g_backend.total_computations > 0) {
        double avg_time = g_backend.computation_time_ms / g_backend.total_computations;
        double avg_energy = (double)g_backend.energy_consumed / g_backend.total_computations;

        printf("║ Temps/opération    : %8.3f ms                           ║\n", avg_time);
        printf("║ Énergie/opération  : %8.2f unités                      ║\n", avg_energy);
    }

    printf("║ Status: %-50s ║\n", g_backend.status_message);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

// Test complet du backend
int lums_backend_comprehensive_test(void) {
    printf("\n=== TEST COMPLET BACKEND LUMS ===\n");

    // Test 1: Fusion
    uint64_t test1_a = 0b11010;  // 3 LUMs
    uint64_t test1_b = 0b1100;   // 2 LUMs
    uint64_t fusion_result;

    if (lums_compute_fusion_real(test1_a, test1_b, &fusion_result) == 0) {
        printf("✓ Test fusion réussi\n");
    } else {
        printf("✗ Test fusion échoué\n");
        return -1;
    }

    // Test 2: Division
    uint64_t split_results[4];
    if (lums_compute_split_real(fusion_result, 3, split_results) == 0) {
        printf("✓ Test division réussi\n");
    } else {
        printf("✗ Test division échoué\n");
        return -2;
    }

    // Test 3: Cycle
    uint64_t cycle_result;
    if (lums_compute_cycle_real(fusion_result, 7, &cycle_result) == 0) {
        printf("✓ Test cycle réussi\n");
    } else {
        printf("✗ Test cycle échoué\n");
        return -3;
    }

    // Test 4: Stockage mémoire
    if (lums_store_memory_real(cycle_result, 5) == 0) {
        printf("✓ Test stockage réussi\n");
    } else {
        printf("✗ Test stockage échoué\n");
        return -4;
    }

    // Test 5: Récupération mémoire
    uint64_t retrieved_data;
    if (lums_retrieve_memory_real(5, &retrieved_data) == 0) {
        printf("✓ Test récupération réussi\n");
    } else {
        printf("✗ Test récupération échoué\n");
        return -5;
    }

    // Test 6: Calcul mathématique
    double sqrt_result = lums_compute_sqrt_via_lums(16.0);
    if (fabs(sqrt_result - 4.0) < 0.001) {
        printf("✓ Test calcul √16 = %.6f réussi\n", sqrt_result);
    } else {
        printf("✗ Test calcul √16 échoué: %.6f\n", sqrt_result);
        return -6;
    }

    // Test 7: Test primalité
    if (lums_test_prime_real(17) == 1) {
        printf("✓ Test primalité 17 réussi\n");
    } else {
        printf("✗ Test primalité 17 échoué\n");
        return -7;
    }

    // Test 8: Fibonacci
    uint64_t fib_results[10];
    if (lums_generate_fibonacci_real(8, fib_results) == 0) {
        printf("✓ Test Fibonacci réussi\n");
    } else {
        printf("✗ Test Fibonacci échoué\n");
        return -8;
    }

    lums_backend_status_report();
    printf("✅ TOUS LES TESTS BACKEND RÉUSSIS\n\n");
    return 0;
}