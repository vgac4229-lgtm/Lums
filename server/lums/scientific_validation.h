#ifndef SCIENTIFIC_VALIDATION_H
#define SCIENTIFIC_VALIDATION_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

// Structure pour les métriques de validation scientifique
typedef struct {
    // Métriques de conservation
    uint64_t conservation_tests_passed;
    uint64_t conservation_tests_total;
    double conservation_accuracy;
    double conservation_entropy_before;
    double conservation_entropy_after;
    
    // Métriques de performance
    double operations_per_second;
    double memory_efficiency;
    double cpu_efficiency;
    double energy_efficiency;
    
    // Métriques de sécurité
    double entropy_quality;
    double cryptographic_strength;
    double resistance_to_attacks;
    
    // Métriques scientifiques
    double mathematical_coherence;
    double physical_plausibility;
    double empirical_evidence;
    
    // Métriques temporelles
    double total_validation_time_ns;
    uint64_t total_validation_cycles;
    struct timespec start_time;
} ScientificValidationMetrics;

// Fonctions de mesure scientifique
void init_scientific_metrics(ScientificValidationMetrics* metrics);
void start_scientific_measurement(ScientificValidationMetrics* metrics);
void end_scientific_measurement(ScientificValidationMetrics* metrics);

// Tests de validation scientifique
int test_conservation_mathematical_rigorous(uint64_t input_a, uint64_t input_b, ScientificValidationMetrics* metrics);
int test_performance_rigorous(ScientificValidationMetrics* metrics);
int test_security_cryptographic_rigorous(ScientificValidationMetrics* metrics);
int test_mathematical_coherence_rigorous(ScientificValidationMetrics* metrics);
int test_physical_plausibility_rigorous(ScientificValidationMetrics* metrics);
int test_empirical_evidence_rigorous(ScientificValidationMetrics* metrics);

// Tests de validation complète
int test_scientific_validation_complete(ScientificValidationMetrics* metrics);
int test_benchmarking_complete(ScientificValidationMetrics* metrics);

#endif // SCIENTIFIC_VALIDATION_H