#include "lums_backend.h"
#include <stdio.h>

int main() {
    printf("=== TEST COMPLET SYSTÈME LUMS/VORAX ===\n");
    
    // Initialisation backend
    if (lums_backend_init() != 0) {
        printf("❌ Échec initialisation backend\n");
        return 1;
    }
    
    // Test complet
    if (lums_backend_comprehensive_test() != 0) {
        printf("❌ Tests échoués\n");
        return 1;
    }
    
    // Rapport final
    lums_backend_status_report();
    
    // Nettoyage
    lums_backend_cleanup();
    
    printf("✅ Tests terminés avec succès\n");
    return 0;
}