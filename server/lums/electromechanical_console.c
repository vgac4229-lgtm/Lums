
#include "electromechanical.h"

void print_system_dashboard(void) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║              SYSTÈME RELAIS ÉLECTROMÉCANIQUES             ║\n");
    printf("║                       LUMS/VORAX                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("║ TICK SYSTÈME: %lu | OPÉRATIONS: %lu                    ║\n",
           g_engine.current_tick, g_engine.total_operations);
    
    printf("╠════════════════════════════════════════════════════════════╣\n");
    
    // État des banks
    for (int i = 0; i < MAX_BANKS; i++) {
        uint8_t active = count_active_relays(i);
        printf("║ Bank %d %-16s: %2d/64 actifs | %d défauts          ║\n",
               i, g_engine.banks[i].name, active, g_engine.banks[i].fault_count);
    }
    
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void print_help_menu(void) {
    printf("\n=== COMMANDES SYSTÈME ÉLECTROMÉCANIQUE ===\n");
    printf("load <bank> <bits>     - Charger pattern binaire\n");
    printf("show <bank>            - Afficher état bank\n");
    printf("fuse <b1> <b2> <res>   - Fusion électromécanique\n");
    printf("split <bank> <parts>   - Division électromécanique\n");
    printf("cycle <bank> <mod>     - Cycle modulo\n");
    printf("flow <src> <dst>       - Flux directionnel\n");
    printf("dashboard              - État système complet\n");
    printf("test                   - Test opérations\n");
    printf("reset                  - Reset système\n");
    printf("quit                   - Arrêt\n");
}

void encode_bits_to_bank(const char* bits, uint8_t bank_id) {
    if (!bits || bank_id >= MAX_BANKS) return;
    
    size_t bit_count = strlen(bits);
    if (bit_count > RELAYS_PER_BANK) bit_count = RELAYS_PER_BANK;
    
    uint64_t state = 0;
    for (size_t i = 0; i < bit_count; i++) {
        if (bits[i] == '1') {
            state |= (1ULL << i);
        }
    }
    
    write_bank_state(bank_id, state);
    printf("✓ Pattern '%s' chargé dans Bank %d\n", bits, bank_id);
}

void show_bank_details(uint8_t bank_id) {
    if (bank_id >= MAX_BANKS) return;
    
    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t state = bank->state;
    uint8_t active = count_active_relays(bank_id);
    
    printf("\n=== BANK %d (%s) ===\n", bank_id, bank->name);
    printf("État: 0x%016llX\n", state);
    printf("Relais actifs: %d/64\n", active);
    printf("Commutations: %u\n", bank->switch_count);
    printf("Défaillants: %d\n", bank->fault_count);
    
    printf("Pattern: ");
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        if (i > 0 && i % 8 == 0) printf(" ");
        printf("%c", (state & (1ULL << i)) ? '1' : '0');
    }
    printf("\n");
}

int run_electromechanical_console(void) {
    ElectromechanicalEngine engine;
    
    if (init_electromechanical_system(&engine) != 0) {
        printf("✗ Échec initialisation système\n");
        return -1;
    }
    
    printf("\n🎛️ CONSOLE ÉLECTROMÉCANIQUE LUMS/VORAX\n");
    printf("Tapez 'help' pour l'aide\n");
    
    char command[256];
    
    while (1) {
        printf("\nLUMS-ELECTRO> ");
        fflush(stdout);
        
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "help") == 0) {
            print_help_menu();
        }
        else if (strncmp(command, "load ", 5) == 0) {
            int bank;
            char bits[65];
            if (sscanf(command + 5, "%d %s", &bank, bits) == 2) {
                encode_bits_to_bank(bits, bank);
            } else {
                printf("Usage: load <bank> <bits>\n");
            }
        }
        else if (strncmp(command, "show ", 5) == 0) {
            int bank;
            if (sscanf(command + 5, "%d", &bank) == 1) {
                show_bank_details(bank);
            } else {
                printf("Usage: show <bank>\n");
            }
        }
        else if (strncmp(command, "fuse ", 5) == 0) {
            int bank1, bank2, result;
            if (sscanf(command + 5, "%d %d %d", &bank1, &bank2, &result) == 3) {
                vorax_fusion_electromechanical(bank1, bank2, result);
            } else {
                printf("Usage: fuse <bank1> <bank2> <result>\n");
            }
        }
        else if (strncmp(command, "split ", 6) == 0) {
            int bank, parts;
            if (sscanf(command + 6, "%d %d", &bank, &parts) == 2) {
                vorax_split_electromechanical(bank, parts);
            } else {
                printf("Usage: split <bank> <parts>\n");
            }
        }
        else if (strncmp(command, "cycle ", 6) == 0) {
            int bank, modulo;
            if (sscanf(command + 6, "%d %d", &bank, &modulo) == 2) {
                vorax_cycle_electromechanical(bank, modulo);
            } else {
                printf("Usage: cycle <bank> <modulo>\n");
            }
        }
        else if (strncmp(command, "flow ", 5) == 0) {
            int src, dst;
            if (sscanf(command + 5, "%d %d", &src, &dst) == 2) {
                vorax_flow_electromechanical(src, dst);
            } else {
                printf("Usage: flow <src> <dst>\n");
            }
        }
        else if (strcmp(command, "dashboard") == 0) {
            print_system_dashboard();
        }
        else if (strcmp(command, "test") == 0) {
            printf("=== TEST SYSTÈME ÉLECTROMÉCANIQUE ===\n");
            encode_bits_to_bank("11010", 0);
            encode_bits_to_bank("1100", 1);
            vorax_fusion_electromechanical(0, 1, 2);
            vorax_split_electromechanical(2, 2);
            vorax_cycle_electromechanical(3, 3);
            printf("✓ Tests terminés\n");
        }
        else if (strcmp(command, "reset") == 0) {
            for (int i = 0; i < MAX_BANKS; i++) {
                write_bank_state(i, 0);
            }
            printf("✓ Système reseté\n");
        }
        else if (strcmp(command, "quit") == 0) {
            printf("Arrêt système électromécanique...\n");
            break;
        }
        else {
            printf("Commande inconnue. Tapez 'help' pour l'aide.\n");
        }
    }
    
    return 0;
}

// Programme principal
int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              SYSTÈME RELAIS ÉLECTROMÉCANIQUES               ║\n");
    printf("║                         LUMS/VORAX                          ║\n");
    printf("║                                                              ║\n");
    printf("║                VERSION ÉLECTROMÉCANIQUE AUTHENTIQUE         ║\n");
    printf("║                                                              ║\n");
    printf("║    Paradigme post-numérique sur hardware électromécanique   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return run_electromechanical_console();
}
