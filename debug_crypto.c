#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Test simple de l'algorithme de signature
int test_signature_algorithm(void) {
    printf("=== DEBUG ALGORITHME DE SIGNATURE ===\n");
    
    // Données de test
    const char* message = "Test Message";
    uint8_t private_key[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                               0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                               0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    
    // Hash du message (simplifié)
    uint8_t message_hash[32];
    for (int i = 0; i < 32; i++) {
        message_hash[i] = message[i % strlen(message)] ^ (i + 1);
    }
    
    printf("Message hash: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", message_hash[i]);
    }
    printf("...\n");
    
    printf("Private key: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", private_key[i]);
    }
    printf("...\n");
    
    // Générer clé publique (hash de la clé privée)
    uint8_t public_key[32];
    for (int i = 0; i < 32; i++) {
        public_key[i] = private_key[i] ^ (i + 1);
    }
    
    printf("Public key: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", public_key[i]);
    }
    printf("...\n");
    
    // Signer
    uint8_t signature[64];
    for (int i = 0; i < 32; i++) {
        signature[i] = message_hash[i] ^ private_key[i];
        signature[i+32] = message_hash[i] ^ (private_key[i] >> 1);
    }
    
    printf("Signature: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", signature[i]);
    }
    printf("...\n");
    
    // Vérifier
    int valid = 1;
    for (int i = 0; i < 32; i++) {
        uint8_t reconstructed_key = message_hash[i] ^ signature[i];
        printf("Byte %d: expected=%02x, got=%02x\n", i, public_key[i], reconstructed_key);
        if (reconstructed_key != public_key[i]) {
            valid = 0;
        }
    }
    
    printf("Signature valide: %s\n", valid ? "OUI" : "NON");
    
    return valid ? 0 : -1;
}

int main(void) {
    if (test_signature_algorithm() == 0) {
        printf("✅ Algorithme de signature fonctionne\n");
    } else {
        printf("❌ Algorithme de signature échoué\n");
    }
    
    return 0;
}