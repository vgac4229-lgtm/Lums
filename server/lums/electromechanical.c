
#include "electromechanical.h"

// Instance globale du moteur
static ElectromechanicalEngine g_engine;
static uint8_t g_initialized = 0;

// Obtenir timestamp nanoseconde
uint64_t get_nanosecond_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000L + ts.tv_nsec);
}

// Délai milliseconde
void delay_ms(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

// Initialisation système électromécanique
int init_electromechanical_system(ElectromechanicalEngine *engine) {
    if (!engine) return -1;
    
    printf("=== INITIALISATION SYSTÈME ÉLECTROMÉCANIQUE LUMS ===\n");
    
    // Initialisation banks
    const char bank_names[][32] = {
        "MEMORY_PRIMARY",
        "MEMORY_SECONDARY", 
        "CALC_TEMPORARY",
        "VORAX_OPERATIONS",
        "INPUT_BUFFER",
        "OUTPUT_BUFFER",
        "SYSTEM_CONTROL",
        "ERROR_RECOVERY"
    };
    
    for (int i = 0; i < MAX_BANKS; i++) {
        engine->banks[i].bank_id = i;
        strcpy(engine->banks[i].name, bank_names[i]);
        engine->banks[i].state = 0;
        engine->banks[i].fault_count = 0;
        engine->banks[i].switch_count = 0;
        engine->banks[i].last_switch_time = 0;
        
        // Initialisation relais défaillants
        memset(engine->banks[i].faulty_relays, 0, 64);
        
        printf("✓ Bank %d (%s) initialisé\n", i, bank_names[i]);
    }
    
    engine->active_banks = MAX_BANKS;
    engine->total_operations = 0;
    engine->current_tick = 0;
    strcpy(engine->error_message, "");
    
    // Test intégrité électrique
    printf("Test intégrité électrique...\n");
    for (int bank = 0; bank < MAX_BANKS; bank++) {
        for (int relay = 0; relay < RELAYS_PER_BANK; relay++) {
            // Simulation test relais
            if ((bank * RELAYS_PER_BANK + relay) % 100 == 99) {
                engine->banks[bank].faulty_relays[relay] = 1;
                engine->banks[bank].fault_count++;
                printf("⚠ Relais %d:%d marqué défaillant\n", bank, relay);
            }
        }
    }
    
    g_engine = *engine;
    g_initialized = 1;
    
    printf("✓ Système électromécanique initialisé : %d banks, %d relais\n", 
           MAX_BANKS, MAX_RELAYS);
    
    return 0;
}

// Contrôle direct relais
void set_relay_state(uint8_t bank, uint8_t position, RelayState state) {
    if (!g_initialized || bank >= MAX_BANKS || position >= RELAYS_PER_BANK) {
        return;
    }
    
    RelayBank *relay_bank = &g_engine.banks[bank];
    
    // Vérifier si relais défaillant
    if (relay_bank->faulty_relays[position]) {
        printf("⚠ Tentative activation relais défaillant %d:%d\n", bank, position);
        return;
    }
    
    // État actuel
    RelayState current = (relay_bank->state & (1ULL << position)) ? RELAY_CLOSED : RELAY_OPEN;
    
    if (current != state) {
        printf("Commutation relais %d:%d %s → %s\n", 
               bank, position,
               current == RELAY_CLOSED ? "FERMÉ" : "OUVERT",
               state == RELAY_CLOSED ? "FERMÉ" : "OUVERT");
        
        // Simulation temps commutation
        delay_ms(RELAY_SWITCHING_TIME_MS);
        
        // Mise à jour état
        if (state == RELAY_CLOSED) {
            relay_bank->state |= (1ULL << position);
        } else {
            relay_bank->state &= ~(1ULL << position);
        }
        
        relay_bank->switch_count++;
        relay_bank->last_switch_time = get_nanosecond_timestamp();
        
        // Stabilisation
        delay_ms(RELAY_SETTLING_TIME_MS);
        
        printf("✓ Relais %d:%d commuté\n", bank, position);
    }
}

// Lecture état relais
RelayState read_relay_state(uint8_t bank, uint8_t position) {
    if (!g_initialized || bank >= MAX_BANKS || position >= RELAYS_PER_BANK) {
        return RELAY_OPEN;
    }
    
    return (g_engine.banks[bank].state & (1ULL << position)) ? RELAY_CLOSED : RELAY_OPEN;
}

// Écriture état complet bank
void write_bank_state(uint8_t bank_id, uint64_t new_state) {
    if (!g_initialized || bank_id >= MAX_BANKS) return;
    
    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t old_state = bank->state;
    
    printf("Bank %d (%s): Écriture état 0x%016llX\n", 
           bank_id, bank->name, new_state);
    
    // Commutation relais un par un
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        RelayState old_bit = (old_state & (1ULL << i)) ? RELAY_CLOSED : RELAY_OPEN;
        RelayState new_bit = (new_state & (1ULL << i)) ? RELAY_CLOSED : RELAY_OPEN;
        
        if (old_bit != new_bit && !bank->faulty_relays[i]) {
            set_relay_state(bank_id, i, new_bit);
        }
    }
    
    bank->state = new_state;
    printf("✓ Bank %d programmé\n", bank_id);
}

// Lecture état complet bank
uint64_t read_bank_state(uint8_t bank_id) {
    if (!g_initialized || bank_id >= MAX_BANKS) return 0;
    return g_engine.banks[bank_id].state;
}

// Compter relais actifs
uint8_t count_active_relays(uint8_t bank_id) {
    if (!g_initialized || bank_id >= MAX_BANKS) return 0;
    
    uint64_t state = g_engine.banks[bank_id].state;
    uint8_t count = 0;
    
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        if (state & (1ULL << i)) count++;
    }
    
    return count;
}

// Vérification conservation
int verify_conservation(uint64_t before, uint64_t after) {
    return (before == after) ? 1 : 0;
}

// FUSION électromécanique
int vorax_fusion_electromechanical(uint8_t bank1, uint8_t bank2, uint8_t result_bank) {
    if (!g_initialized) return -1;
    
    printf("FUSION ÉLECTROMÉCANIQUE: Bank%d + Bank%d → Bank%d\n", 
           bank1, bank2, result_bank);
    
    uint64_t state1 = read_bank_state(bank1);
    uint64_t state2 = read_bank_state(bank2);
    uint8_t count1 = count_active_relays(bank1);
    uint8_t count2 = count_active_relays(bank2);
    
    // Conservation : total avant = total après
    uint8_t total_before = count1 + count2;
    
    // Fusion par concaténation
    uint64_t fused_state = 0;
    uint8_t bit_pos = 0;
    
    // Copie bank1
    for (int i = 0; i < RELAYS_PER_BANK && bit_pos < RELAYS_PER_BANK; i++) {
        if (state1 & (1ULL << i)) {
            fused_state |= (1ULL << bit_pos);
            bit_pos++;
        }
    }
    
    // Copie bank2
    for (int i = 0; i < RELAYS_PER_BANK && bit_pos < RELAYS_PER_BANK; i++) {
        if (state2 & (1ULL << i)) {
            fused_state |= (1ULL << bit_pos);
            bit_pos++;
        }
    }
    
    // Programmation résultat
    write_bank_state(result_bank, fused_state);
    
    // Vérification conservation
    uint8_t total_after = count_active_relays(result_bank);
    if (!verify_conservation(total_before, total_after)) {
        printf("✗ VIOLATION CONSERVATION: %d → %d\n", total_before, total_after);
        return -2;
    }
    
    g_engine.total_operations++;
    printf("✓ Fusion réussie: %d + %d → %d relais\n", count1, count2, total_after);
    
    return 0;
}

// DIVISION électromécanique
int vorax_split_electromechanical(uint8_t source_bank, uint8_t target_count) {
    if (!g_initialized || target_count < 2 || target_count > 4) return -1;
    
    printf("DIVISION ÉLECTROMÉCANIQUE: Bank%d → %d parts\n", 
           source_bank, target_count);
    
    uint64_t source_state = read_bank_state(source_bank);
    uint8_t total_active = count_active_relays(source_bank);
    uint8_t per_bank = total_active / target_count;
    uint8_t remainder = total_active % target_count;
    
    // Distribution dans banks suivants
    uint8_t current_bit = 0;
    for (uint8_t i = 0; i < target_count; i++) {
        uint8_t target_bank = source_bank + 1 + i;
        if (target_bank >= MAX_BANKS) break;
        
        uint8_t bank_size = per_bank + (i < remainder ? 1 : 0);
        uint64_t bank_state = 0;
        uint8_t assigned = 0;
        
        // Attribution relais
        for (int j = 0; j < RELAYS_PER_BANK && assigned < bank_size; j++) {
            if (source_state & (1ULL << j)) {
                if (current_bit-- == 0) {
                    bank_state |= (1ULL << assigned);
                    assigned++;
                }
            }
        }
        
        write_bank_state(target_bank, bank_state);
        printf("  Bank%d: %d relais\n", target_bank, assigned);
    }
    
    // Effacement source
    write_bank_state(source_bank, 0);
    
    g_engine.total_operations++;
    printf("✓ Division terminée\n");
    
    return 0;
}

// CYCLE électromécanique
int vorax_cycle_electromechanical(uint8_t bank, uint8_t modulo) {
    if (!g_initialized || modulo == 0) return -1;
    
    printf("CYCLE ÉLECTROMÉCANIQUE: Bank%d modulo %d\n", bank, modulo);
    
    uint8_t active_count = count_active_relays(bank);
    uint8_t cycled_count = active_count % modulo;
    
    if (cycled_count == 0 && active_count > 0) {
        cycled_count = modulo;
    }
    
    // Nouvelle configuration avec premiers N relais
    uint64_t cycled_state = 0;
    for (uint8_t i = 0; i < cycled_count; i++) {
        cycled_state |= (1ULL << i);
    }
    
    write_bank_state(bank, cycled_state);
    
    g_engine.total_operations++;
    printf("✓ Cycle: %d → %d relais\n", active_count, cycled_count);
    
    return 0;
}

// FLUX électromécanique
int vorax_flow_electromechanical(uint8_t source_bank, uint8_t target_bank) {
    if (!g_initialized) return -1;
    
    printf("FLUX ÉLECTROMÉCANIQUE: Bank%d → Bank%d\n", source_bank, target_bank);
    
    uint64_t source_state = read_bank_state(source_bank);
    uint64_t target_state = read_bank_state(target_bank);
    
    // Transfert avec animation
    for (int step = 0; step < 5; step++) {
        printf("  Transfert... %d%%\n", (step + 1) * 20);
        delay_ms(100);
    }
    
    // Application transfert
    write_bank_state(target_bank, source_state | target_state);
    write_bank_state(source_bank, 0);
    
    g_engine.total_operations++;
    printf("✓ Flux terminé\n");
    
    return 0;
}
