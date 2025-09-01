#include "electromechanical.h"
#include <stdio.h>

// Implementation des fonctions manquantes pour le backend

int electromechanical_engine_init(ElectromechanicalEngine* engine) {
    if (!engine) return -1;
    
    printf("🔧 Initialisation moteur électromécanique...\n");
    
    // Initialize with init_electromechanical_system
    int result = init_electromechanical_system(engine);
    if (result == 0) {
        printf("✓ Moteur électromécanique initialisé\n");
    }
    return result;
}

void electromechanical_engine_cleanup(ElectromechanicalEngine* engine) {
    if (!engine) return;
    
    printf("🧹 Nettoyage moteur électromécanique...\n");
    
    // Reset all relay states
    for (int bank = 0; bank < MAX_BANKS; bank++) {
        write_bank_state(bank, 0);
    }
    
    printf("✓ Moteur électromécanique nettoyé\n");
}

int electromechanical_operate_relay(ElectromechanicalEngine* engine, int relay_id, int state) {
    if (!engine) return -1;
    
    uint8_t bank = relay_id / RELAYS_PER_BANK;
    uint8_t position = relay_id % RELAYS_PER_BANK;
    
    if (bank >= MAX_BANKS) return -2;
    
    set_relay_state(bank, position, state ? RELAY_CLOSED : RELAY_OPEN);
    return 0;
}