#ifndef LUMS_H
#define LUMS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Version et métadonnées
#define LUMS_VERSION_MAJOR 1
#define LUMS_VERSION_MINOR 0
#define LUMS_VERSION_PATCH 0
#define LUMS_VERSION_STRING "1.0.0-validated"

// Constantes système
#define LUMS_MAX_ZONES 16
#define LUMS_MAX_LUM_COUNT 1024
#define LUMS_CONSERVATION_THRESHOLD 0.9999
#define LUMS_ENTROPY_SCALING_FACTOR 1e-6

// Types de base LUMS
typedef uint64_t LUM;
typedef uint32_t ZoneID;
typedef uint32_t LUMCount;

// Énumération des types de structure LUM
typedef enum {
    LUM_LINEAR = 0,
    LUM_CLUSTERED = 1,
    LUM_MEMORY = 2,
    LUM_NODE = 3,
    LUM_CYCLE = 4
} LUMStructureType;

// Structure pour position 2D
typedef struct {
    int x;
    int y;
} LUMPosition;

// Structure LUM de base
typedef struct {
    LUM value;                    // Valeur LUM (64 bits)
    double presence;              // Facteur de présence [0.0, 1.0]
    uint32_t zone_id;            // Identifiant de zone
    uint64_t timestamp_ns;       // Timestamp nanoseconde
    LUMStructureType structure_type; // Type de structure
    LUMPosition position;        // Position spatiale
    void* spatial_data;          // Données spatiales additionnelles
} LUMStruct;

// Groupe de LUMs
typedef struct {
    LUMStruct* lums;             // Tableau de LUMs
    size_t count;                // Nombre de LUMs
    size_t capacity;             // Capacité allouée
    double total_conservation;   // Conservation totale
    uint64_t group_id;          // Identifiant unique du groupe
} LUMGroup;

// Zone VORAX
typedef struct {
    LUMGroup* group;             // Groupe de LUMs dans cette zone
    uint32_t zone_id;           // Identifiant de zone
    char name[64];              // Nom de la zone
    bool is_active;             // État d'activité
    uint64_t last_operation_ns; // Dernière opération
} VoraxZone;

// Moteur VORAX
typedef struct {
    VoraxZone* zones;           // Tableau de zones
    size_t zone_count;          // Nombre de zones
    size_t max_zones;           // Nombre maximum de zones
    uint64_t total_operations;  // Nombre total d'opérations
    double global_conservation; // Conservation globale
} VoraxEngine;

// Métriques de performance
typedef struct {
    uint64_t operations_count;   // Nombre d'opérations
    uint64_t total_cycles;      // Cycles CPU totaux
    uint64_t total_time_ns;     // Temps total en nanosecondes
    double average_conservation; // Conservation moyenne
    size_t memory_usage_bytes;  // Utilisation mémoire
} PerformanceMetrics;

// Types d'opération VORAX
typedef enum {
    VORAX_OP_FUSION = 0,
    VORAX_OP_SPLIT = 1,
    VORAX_OP_CYCLE = 2,
    VORAX_OP_TRANSFER = 3,
    VORAX_OP_VALIDATE = 4
} VoraxOperationType;

// Résultat d'opération
typedef struct {
    VoraxOperationType type;     // Type d'opération
    bool success;               // Succès de l'opération
    double conservation_factor; // Facteur de conservation
    uint64_t execution_time_ns; // Temps d'exécution
    char error_message[256];    // Message d'erreur si échec
} OperationResult;

// ============================================================================
// DÉCLARATIONS DES FONCTIONS PRINCIPALES
// ============================================================================

// Initialisation et nettoyage
int lums_init(void);
void lums_cleanup(void);

// Gestion des groupes LUM
LUMGroup* lums_group_create(size_t initial_capacity);
void lums_group_destroy(LUMGroup* group);
int lums_group_add(LUMGroup* group, LUM value, double presence);
bool lums_group_validate_conservation(const LUMGroup* group);

// Opérations VORAX de base
int lums_compute_fusion_real(uint64_t lum_a, uint64_t lum_b, uint64_t* result);
int lums_compute_split_real(uint64_t lum_source, uint64_t* result_a, uint64_t* result_b);
int lums_compute_cycle_real(uint64_t lum_input, int modulo, uint64_t* result);

// Moteur VORAX
VoraxEngine* vorax_engine_create(size_t max_zones);
void vorax_engine_destroy(VoraxEngine* engine);
int vorax_engine_add_zone(VoraxEngine* engine, const char* name);
OperationResult vorax_execute_operation(VoraxEngine* engine, VoraxOperationType type, 
                                       uint32_t zone_a, uint32_t zone_b, uint32_t target_zone);

// Validation et conservation
bool lums_validate_conservation_law(const LUMGroup* before, const LUMGroup* after);
double lums_calculate_entropy(const LUMGroup* group);
PerformanceMetrics lums_get_performance_metrics(void);

// Utilitaires de conversion
char* lums_to_binary_string(LUM value);
LUM lums_from_binary_string(const char* binary_str);
uint32_t lums_count_active_bits(LUM value);

// Fonctions de logging scientifique
void lums_log_operation(VoraxOperationType type, uint64_t input_a, uint64_t input_b, 
                       uint64_t result, uint64_t timestamp_ns);
void lums_log_conservation(double before, double after, bool valid);

// Constantes mathématiques
extern const double LUMS_PI;
extern const double LUMS_E;
extern const uint64_t LUMS_PRIME_CONSTANT;

#endif // LUMS_H