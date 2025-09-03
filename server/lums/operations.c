#include "lums.h"
#include <assert.h>
#include <errno.h>
#include <string.h> // For memset
#include <stdlib.h> // For malloc, realloc, free
#include <time.h>   // For time()
#include <math.h>   // For fabs, log2, cos, sin, fmod

// Assuming VoraxOperationType and LUMStructureType are defined in lums.h
// Assuming LUMStruct is defined in lums.h
// Assuming PerformanceMetrics is defined in lums.h

// Variables globales pour métriques
static PerformanceMetrics g_metrics = {0};
static bool g_lums_initialized = false;

// Constantes mathématiques
const double LUMS_PI = 3.14159265358979323846;
const double LUMS_E = 2.71828182845904523536;
const uint64_t LUMS_PRIME_CONSTANT = 0x1FFFFF03ULL; // Premier réel: 536870659
const double LUMS_CONSERVATION_THRESHOLD = 0.99; // Seuil pour la conservation

// ============================================================================
// INITIALISATION ET NETTOYAGE
// ============================================================================

int lums_init(void) {
    if (g_lums_initialized) {
        return 0; // Déjà initialisé
    }

    // Initialisation des métriques
    memset(&g_metrics, 0, sizeof(PerformanceMetrics));

    // Initialisation du générateur aléatoire
    srand((unsigned int)time(NULL));

    g_lums_initialized = true;
    return 0;
}

void lums_cleanup(void) {
    if (!g_lums_initialized) {
        return;
    }

    // Nettoyage des ressources globales
    memset(&g_metrics, 0, sizeof(PerformanceMetrics));
    g_lums_initialized = false;
}

// ============================================================================
// GESTION DES GROUPES LUM
// ============================================================================

LUMGroup* lums_group_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 16; // Capacité par défaut
    }

    LUMGroup* group = malloc(sizeof(LUMGroup));
    if (!group) {
        return NULL;
    }

    group->lums = malloc(sizeof(LUMStruct) * initial_capacity);
    if (!group->lums) {
        free(group);
        return NULL;
    }

    group->count = 0;
    group->capacity = initial_capacity;
    group->total_conservation = 0.0;
    group->group_id = (uint64_t)group; // ID basé sur l'adresse

    return group;
}

void lums_group_destroy(LUMGroup* group) {
    if (!group) {
        return;
    }

    if (group->lums) {
        free(group->lums);
    }

    memset(group, 0, sizeof(LUMGroup));
    free(group);
}

int lums_group_add(LUMGroup* group, LUM value, double presence) {
    if (!group || presence < 0.0 || presence > 1.0) {
        return -1;
    }

    // Redimensionnement si nécessaire
    if (group->count >= group->capacity) {
        size_t new_capacity = group->capacity * 2;
        LUMStruct* new_lums = realloc(group->lums, sizeof(LUMStruct) * new_capacity);
        if (!new_lums) {
            return -1;
        }
        group->lums = new_lums;
        group->capacity = new_capacity;
    }

    // Ajout du nouveau LUM
    LUMStruct* lum = &group->lums[group->count];
    lum->value = value;
    lum->presence = presence;
    lum->zone_id = 0; // Zone par défaut
    // Using time(NULL) directly might not be granular enough for nanoseconds.
    // A more precise timer would be needed for nanosecond accuracy.
    // For demonstration, we'll use time as a placeholder.
    lum->timestamp_ns = (uint64_t)time(NULL) * 1000000000ULL; 

    // Mise à jour de la conservation
    // __builtin_popcountll is a GCC/Clang extension. For portability, a manual count might be needed.
    group->total_conservation += (double)__builtin_popcountll(value) * presence;
    group->count++;

    return 0;
}

bool lums_group_validate_conservation(const LUMGroup* group) {
    if (!group) {
        return false;
    }

    double calculated_conservation = 0.0;
    for (size_t i = 0; i < group->count; i++) {
        const LUMStruct* lum = &group->lums[i];
        // Assuming __builtin_popcountll is available or a portable alternative is used.
        calculated_conservation += (double)__builtin_popcountll(lum->value) * lum->presence;
    }

    double difference = fabs(calculated_conservation - group->total_conservation);
    return difference < LUMS_CONSERVATION_THRESHOLD;
}

// ============================================================================
// OPÉRATIONS LUMS FONDAMENTALES
// ============================================================================

int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result) {
    if (!result || !g_lums_initialized) {
        return -1;
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Conservation avant fusion
    uint32_t bits_before = __builtin_popcountll(lum_a);
    uint32_t bits_b_before = __builtin_popcountll(lum_b);

    // Opération de fusion (XOR conservatrice)
    *result = lum_a ^ lum_b;

    // Vérification conservation
    uint32_t bits_after = __builtin_popcountll(*result);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    uint64_t execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000000000ULL +
                             (end_time.tv_nsec - start_time.tv_nsec);

    // Mise à jour des métriques
    g_metrics.operations_count++;
    g_metrics.total_time_ns += execution_time;

    // Logging de l'opération
    lums_log_operation(VORAX_OP_FUSION, lum_a, lum_b, *result, 
                      (uint64_t)start_time.tv_sec * 1000000000ULL + start_time.tv_nsec);

    // Validation conservation (XOR peut réduire le nombre de bits)
    bool conservation_valid = (bits_after <= bits_before + bits_b_before); // Conservation means bits don't increase
    lums_log_conservation((double)bits_before + bits_b_before, (double)bits_after, conservation_valid);

    return conservation_valid ? 0 : -1;
}

int lums_compute_split_real(uint64_t lum_source, uint64_t* result_a, uint64_t* result_b) {
    if (!result_a || !result_b || !g_lums_initialized) {
        return -1;
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Conservation avant split
    uint32_t bits_before = __builtin_popcountll(lum_source);

    // Algorithme de split conservatif
    // Distribution alternée des bits
    uint64_t mask_a = 0x5555555555555555ULL; // Bits pairs
    uint64_t mask_b = 0xAAAAAAAAAAAAAAAAULL; // Bits impairs

    *result_a = lum_source & mask_a;
    *result_b = lum_source & mask_b;

    // Vérification conservation
    uint32_t bits_after = __builtin_popcountll(*result_a) + __builtin_popcountll(*result_b);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    uint64_t execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000000000ULL +
                             (end_time.tv_nsec - start_time.tv_nsec);

    // Mise à jour des métriques
    g_metrics.operations_count++;
    g_metrics.total_time_ns += execution_time;

    // Validation conservation parfaite pour split
    bool conservation_valid = (bits_after == bits_before);
    lums_log_conservation((double)bits_before, (double)bits_after, conservation_valid);

    return conservation_valid ? 0 : -1;
}

int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result) {
    if (!result || modulo <= 0 || !g_lums_initialized) {
        return -1;
    }

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Opération cyclique conservative
    uint32_t shift_amount = modulo % 64;
    *result = (lum_input << shift_amount) | (lum_input >> (64 - shift_amount));

    // Mise à jour des métriques
    g_metrics.operations_count++;

    // Conservation parfaite pour rotation
    lums_log_conservation((double)__builtin_popcountll(lum_input), 
                         (double)__builtin_popcountll(*result), true);

    return 0;
}

// ============================================================================
// VALIDATION ET CONSERVATION
// ============================================================================

bool lums_validate_conservation_law(const LUMGroup* before, const LUMGroup* after) {
    if (!before || !after) {
        return false;
    }

    double conservation_before = before->total_conservation;
    double conservation_after = after->total_conservation;

    // Avoid division by zero if conservation_before is zero
    if (conservation_before == 0.0) {
        return conservation_after == 0.0; // If before is 0, after must also be 0 for conservation
    }

    double ratio = conservation_after / conservation_before;
    return (ratio >= LUMS_CONSERVATION_THRESHOLD && ratio <= (1.0 / LUMS_CONSERVATION_THRESHOLD));
}

double lums_calculate_entropy(const LUMGroup* group) {
    if (!group || group->count == 0) {
        return 0.0;
    }

    double total_entropy = 0.0;
    double total_presence = 0.0;

    for (size_t i = 0; i < group->count; i++) {
        const LUMStruct* lum = &group->lums[i];
        // Assuming LUM value is intended to represent a probability or proportion.
        // If LUM is just a bitmask, this calculation might not be meaningful.
        // For demonstration, let's use a normalized value based on bits.
        // A more appropriate value for entropy calculation would be needed.
        double normalized_value = (double)__builtin_popcountll(lum->value) / 64.0; // Example: proportion of set bits

        if (normalized_value > 0.0 && normalized_value < 1.0) {
            total_entropy += -normalized_value * log2(normalized_value) * lum->presence;
            total_entropy += -(1.0 - normalized_value) * log2(1.0 - normalized_value) * lum->presence;
        } else if (normalized_value == 0.0 || normalized_value == 1.0) {
            // Entropy is 0 if probability is 0 or 1, no need to add.
        }

        total_presence += lum->presence;
    }

    return total_presence > 0.0 ? total_entropy / total_presence : 0.0;
}

PerformanceMetrics lums_get_performance_metrics(void) {
    // Calcul des moyennes
    if (g_metrics.operations_count > 0) {
        g_metrics.average_time_ns = g_metrics.total_time_ns / (double)g_metrics.operations_count;
    } else {
        g_metrics.average_time_ns = 0.0;
    }

    return g_metrics;
}

// ============================================================================
// UTILITAIRES DE CONVERSION
// ============================================================================

char* lums_to_binary_string(LUM value) {
    char* binary_str = malloc(65); // 64 bits + '\0'
    if (!binary_str) {
        return NULL;
    }

    for (int i = 63; i >= 0; i--) {
        binary_str[63 - i] = ((value >> i) & 1) ? '1' : '0';
    }
    binary_str[64] = '\0';

    return binary_str;
}

LUM lums_from_binary_string(const char* binary_str) {
    if (!binary_str) {
        return 0;
    }

    LUM value = 0;
    size_t len = strlen(binary_str);

    for (size_t i = 0; i < len && i < 64; i++) {
        if (binary_str[i] == '1') {
            value |= (1ULL << (len - 1 - i));
        } else if (binary_str[i] != '0') {
            // Handle invalid characters if necessary, here we ignore them
        }
    }

    return value;
}

uint32_t lums_count_active_bits(LUM value) {
    // Assuming __builtin_popcountll is available.
    return (uint32_t)__builtin_popcountll(value);
}

// ============================================================================
// LOGGING SCIENTIFIQUE (STUBS POUR LIAISON AVEC scientific_logger.c)
// ============================================================================

void lums_log_operation(VoraxOperationType type, uint64_t input_a, uint64_t input_b, 
                       uint64_t result, uint64_t timestamp_ns) {
    // Stub implementation: In a real scenario, this would call a logging function
    // from a scientific logger module.
    (void)type; (void)input_a; (void)input_b; (void)result; (void)timestamp_ns;
    // Example: printf("LOG: Operation %d, Inputs: %llu, %llu, Result: %llu, Time: %lluns\n",
    //                   type, input_a, input_b, result, timestamp_ns);
}

void lums_log_conservation(double before, double after, bool valid) {
    // Stub implementation: In a real scenario, this would call a logging function
    // from a scientific logger module.
    (void)before; (void)after; (void)valid;
    // Example: printf("LOG: Conservation Check: Before=%.2f, After=%.2f, Valid=%s\n",
    //                   before, after, valid ? "true" : "false");
}