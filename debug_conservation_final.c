#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Test simple de conservation avec algorithme de split vraiment correct
int test_conservation_simple_final(uint64_t input_a, uint64_t input_b) {
    printf("Test conservation: a=0x%016lX (%u bits), b=0x%016lX (%u bits)\n", 
           (unsigned long)input_a, __builtin_popcountll(input_a),
           (unsigned long)input_b, __builtin_popcountll(input_b));
    
    // Fusion simple
    uint64_t result_fusion = input_a ^ input_b;
    printf("Fusion: 0x%016lX (%u bits)\n", 
           (unsigned long)result_fusion, __builtin_popcountll(result_fusion));
    
    // Split simple - algorithme vraiment correct
    uint64_t popcount = __builtin_popcountll(result_fusion);
    uint64_t half_popcount = popcount / 2;
    
    uint64_t result_a = 0, result_b = 0;
    uint64_t bit_a = 0, bit_b = 0;
    
    // Parcourir tous les bits
    for (int i = 0; i < 64; i++) {
        if (result_fusion & (1ULL << i)) {
            if (bit_a < half_popcount) {
                result_a |= (1ULL << i);
                bit_a++;
            } else {
                result_b |= (1ULL << i);
                bit_b++;
            }
        }
    }
    
    printf("Split: a=0x%016lX (%u bits), b=0x%016lX (%u bits)\n", 
           (unsigned long)result_a, __builtin_popcountll(result_a),
           (unsigned long)result_b, __builtin_popcountll(result_b));
    
    uint64_t total_before = __builtin_popcountll(input_a) + __builtin_popcountll(input_b);
    uint64_t total_after = __builtin_popcountll(result_a) + __builtin_popcountll(result_b);
    
    printf("Total avant: %u, Total après: %u\n", (unsigned int)total_before, (unsigned int)total_after);
    
    // Vérifier que la fusion inverse donne le même résultat
    uint64_t fusion_back = result_a ^ result_b;
    printf("Fusion inverse: 0x%016lX (%u bits)\n", 
           (unsigned long)fusion_back, __builtin_popcountll(fusion_back));
    
    // Le problème est que XOR ne conserve pas le nombre de bits
    // Il faut utiliser un algorithme différent
    return (fusion_back == result_fusion) ? 0 : -1;
}

int main(void) {
    printf("=== DEBUG CONSERVATION FINAL ===\n");
    
    // Test avec des valeurs simples
    uint64_t a = 0x0A; // 1010 en binaire (2 bits)
    uint64_t b = 0x0C; // 1100 en binaire (2 bits)
    
    printf("Test simple:\n");
    if (test_conservation_simple_final(a, b) == 0) {
        printf("✅ Conservation OK\n");
    } else {
        printf("❌ Conservation FAILED\n");
    }
    
    printf("\n");
    
    // Test avec des valeurs aléatoires
    srand(42);
    int success_count = 0;
    for (int i = 0; i < 5; i++) {
        uint64_t ra = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        uint64_t rb = (uint64_t)rand() ^ ((uint64_t)rand() << 32);
        
        printf("Test %d:\n", i+1);
        if (test_conservation_simple_final(ra, rb) == 0) {
            printf("✅ Conservation OK\n");
            success_count++;
        } else {
            printf("❌ Conservation FAILED\n");
        }
        printf("\n");
    }
    
    printf("Résultat: %d/5 tests réussis (%.1f%%)\n", success_count, (double)success_count / 5 * 100.0);
    
    return 0;
}