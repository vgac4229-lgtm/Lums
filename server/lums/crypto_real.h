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
#ifndef CRYPTO_REAL_H
#define CRYPTO_REAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Structure pour métriques cryptographiques
typedef struct {
    uint64_t operations_count;
    uint64_t total_time_ns;
    uint64_t entropy_bytes_generated;
    uint64_t hash_operations;
    uint64_t signature_operations;
} CryptoRealMetrics;

// ============================================================================
// FONCTIONS D'INITIALISATION
// ============================================================================

/**
 * Initialise le sous-système cryptographique
 * @return 0 en cas de succès, -1 en cas d'erreur
 */
int crypto_real_init(void);

/**
 * Nettoie le sous-système cryptographique
 */
void crypto_real_cleanup(void);

// ============================================================================
// GÉNÉRATION D'ENTROPIE
// ============================================================================

/**
 * Génère de l'entropie cryptographique réelle via le système
 * @param buffer Buffer de sortie
 * @param len Nombre d'octets à générer
 * @param metrics Métriques optionnelles (peut être NULL)
 * @return 0 en cas de succès, -1 en cas d'erreur
 */
int generate_crypto_entropy_real(uint8_t *buffer, size_t len, CryptoRealMetrics* metrics);

// ============================================================================
// FONCTIONS DE HACHAGE
// ============================================================================

/**
 * Calcule SHA-3-256 authentique via OpenSSL
 * @param data Données à hacher
 * @param len Longueur des données
 * @param out Buffer de sortie (32 bytes)
 * @param metrics Métriques optionnelles (peut être NULL)
 * @return 0 en cas de succès, -1 en cas d'erreur
 */
int compute_sha3_256_real(const void *data, size_t len, unsigned char out[32], CryptoRealMetrics* metrics);

// ============================================================================
// SIGNATURES CRYPTOGRAPHIQUES
// ============================================================================

/**
 * Génère une paire de clés simple
 * @param public_key Clé publique de sortie (32 bytes)
 * @param private_key Clé privée de sortie (32 bytes)
 * @param metrics Métriques optionnelles (peut être NULL)
 * @return 0 en cas de succès, -1 en cas d'erreur
 */
int generate_simple_keypair(uint8_t public_key[32], uint8_t private_key[32], CryptoRealMetrics* metrics);

/**
 * Signe un message avec une clé privée
 * @param message Message à signer
 * @param message_len Longueur du message
 * @param private_key Clé privée (32 bytes)
 * @param signature Signature de sortie (64 bytes)
 * @param metrics Métriques optionnelles (peut être NULL)
 * @return 0 en cas de succès, -1 en cas d'erreur
 */
int sign_simple(const uint8_t *message, size_t message_len, const uint8_t private_key[32], 
                uint8_t signature[64], CryptoRealMetrics* metrics);

/**
 * Vérifie une signature
 * @param message Message original
 * @param message_len Longueur du message
 * @param signature Signature à vérifier (64 bytes)
 * @param public_key Clé publique (32 bytes)
 * @param metrics Métriques optionnelles (peut être NULL)
 * @return 0 si signature valide, -1 sinon
 */
int verify_simple(const uint8_t *message, size_t message_len, const uint8_t signature[64], 
                  const uint8_t public_key[32], CryptoRealMetrics* metrics);

// ============================================================================
// MÉTRIQUES ET DIAGNOSTICS
// ============================================================================

/**
 * Récupère les métriques cryptographiques
 * @return Structure contenant les métriques
 */
CryptoRealMetrics crypto_real_get_metrics(void);

/**
 * Remet à zéro les métriques
 */
void crypto_real_reset_metrics(void);

#endif // CRYPTO_REAL_H
