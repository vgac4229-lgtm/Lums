#ifndef CRYPTO_REAL_H
#define CRYPTO_REAL_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

// Structure pour les métriques cryptographiques réelles
typedef struct {
    // Métriques SHA-3
    double sha3_time_ns;
    uint64_t sha3_cycles;
    size_t sha3_input_size;
    size_t sha3_output_size;
    
    // Métriques entropie
    double entropy_time_ns;
    uint64_t entropy_cycles;
    size_t entropy_bytes_generated;
    double entropy_quality; // Mesure de qualité de l'entropie
    
    // Métriques signatures
    double signature_time_ns;
    uint64_t signature_cycles;
    size_t signature_size;
    int signature_valid;
    
    // Métriques globales
    double total_time_ns;
    uint64_t total_cycles;
    size_t total_memory_used;
    struct timespec start_time;
} CryptoRealMetrics;

// Fonctions de mesure cryptographique
void init_crypto_metrics(CryptoRealMetrics* metrics);
void start_crypto_measurement(CryptoRealMetrics* metrics);
void end_crypto_measurement(CryptoRealMetrics* metrics);

// Fonctions SHA-3 réelles
int compute_sha3_256_real(const void *data, size_t len, unsigned char out[32], CryptoRealMetrics* metrics);

// Fonctions entropie cryptographique réelles
int generate_crypto_entropy_real(uint8_t *buffer, size_t len, CryptoRealMetrics* metrics);

// Fonctions signatures Ed25519 réelles
int generate_ed25519_keypair_real(uint8_t public_key[32], uint8_t private_key[32], CryptoRealMetrics* metrics);
int sign_ed25519_real(const uint8_t *message, size_t message_len,
                     const uint8_t private_key[32], uint8_t signature[64], CryptoRealMetrics* metrics);
int verify_ed25519_real(const uint8_t *message, size_t message_len,
                       const uint8_t signature[64], const uint8_t public_key[32], CryptoRealMetrics* metrics);

// Tests de validation cryptographique
int test_crypto_performance_real(void);
int test_crypto_validation_real(void);

#endif // CRYPTO_REAL_H