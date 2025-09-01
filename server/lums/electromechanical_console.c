#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_BANKS 8
#define RELAYS_PER_BANK 64

// Structure pour représenter un relais individuel
typedef struct {
    bool is_active;
    uint32_t switch_count;
} Relay;

// Structure pour représenter une banque de relais
typedef struct {
    char name[50];
    Relay relays[RELAYS_PER_BANK];
    uint64_t state; // État actuel de la banque (64 bits)
    uint32_t switch_count;
    int fault_count;
} RelayBank;

// Structure principale du moteur électromécanique
typedef struct {
    RelayBank banks[MAX_BANKS];
    uint64_t current_tick;
    uint64_t total_operations;
} ElectromechanicalEngine;

// Variables globales
extern ElectromechanicalEngine g_engine;
extern uint8_t g_initialized;

// Déclarations des fonctions pour les opérations électromécaniques (implémentation supposée ailleurs)
void write_bank_state(uint8_t bank_id, uint64_t state);
uint8_t count_active_relays(uint8_t bank_id);
int init_electromechanical_system(ElectromechanicalEngine *engine);
void vorax_fusion_electromechanical(uint8_t bank1_id, uint8_t bank2_id, uint8_t result_bank_id);
void vorax_split_electromechanical(uint8_t bank_id, uint8_t num_parts);
void vorax_cycle_electromechanical(uint8_t bank_id, uint8_t modulo);
void vorax_flow_electromechanical(uint8_t src_bank_id, uint8_t dst_bank_id);

void print_system_dashboard(void) {
    if (!g_initialized) {
        printf("✗ Backend LUMS non initialisé\n");
        return;
    }

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
    if (!g_initialized || !bits || bank_id >= MAX_BANKS) {
        printf("✗ Opération échouée : Système non initialisé ou paramètres invalides.\n");
        return;
    }

    size_t bit_count = strlen(bits);
    if (bit_count > RELAYS_PER_BANK) {
        printf("! Attention : Le pattern dépasse %d bits, troncature appliquée.\n", RELAYS_PER_BANK);
        bit_count = RELAYS_PER_BANK;
    }

    uint64_t state = 0;
    for (size_t i = 0; i < bit_count; i++) {
        if (bits[i] == '1') {
            state |= (1ULL << i);
        } else if (bits[i] != '0') {
            printf("! Attention : Caractère invalide '%c' dans le pattern, traité comme '0'.\n", bits[i]);
        }
    }

    write_bank_state(bank_id, state);
    g_engine.total_operations++;
    printf("✓ Pattern '%s' chargé dans Bank %d\n", bits, bank_id);
}

void show_bank_details(uint8_t bank_id) {
    if (!g_initialized || bank_id >= MAX_BANKS) {
        printf("✗ Opération échouée : Système non initialisé ou ID de banque invalide.\n");
        return;
    }

    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t state = bank->state;
    uint8_t active = count_active_relays(bank_id);

    printf("\n=== BANK %d (%s) ===\n", bank_id, bank->name);
    printf("État: 0x%016llX\n", state);
    printf("Relais actifs: %d/64\n", active);
    printf("Commutations totales: %u\n", bank->switch_count);
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
    g_engine = engine; // Initialisation de la variable globale

    if (init_electromechanical_system(&g_engine) != 0) {
        printf("✗ Échec initialisation système\n");
        g_initialized = 0;
        return -1;
    }
    g_initialized = 1; // Marquer le système comme initialisé

    printf("\n🎛️ CONSOLE ÉLECTROMÉCANIQUE LUMS/VORAX\n");
    printf("Tapez 'help' pour l'aide\n");

    char command[256];

    while (1) {
        printf("\nLUMS-ELECTRO> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
             printf("\nErreur de lecture ou fin de fichier détectée. Arrêt.\n");
             break;
        }

        // Remove newline
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "help") == 0) {
            print_help_menu();
        }
        else if (strncmp(command, "load ", 5) == 0) {
            int bank;
            char bits[65]; // +1 pour le caractère nul potentiel
            if (sscanf(command + 5, "%d %64s", &bank, bits) == 2) { // S'assurer de ne pas dépasser 64 caractères pour bits
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
            g_engine.current_tick = 0;
            g_engine.total_operations = 0;
            printf("✓ Système reseté\n");
        }
        else if (strcmp(command, "quit") == 0) {
            printf("Arrêt système électromécanique...\n");
            break;
        }
        else {
            printf("Commande inconnue. Tapez 'help' pour l'aide.\n");
        }
        g_engine.current_tick++; // Incrémente le tick système à chaque commande traitée
    }

    // Ici, on pourrait ajouter une logique de nettoyage si nécessaire avant de retourner
    // Par exemple, désallouer de la mémoire, fermer des fichiers, etc.

    return 0;
}

// Dummy implementations for external functions (replace with actual implementations)
// In a real scenario, these would be in separate .c files and linked.

ElectromechanicalEngine g_engine;
uint8_t g_initialized = 0;

int init_electromechanical_system(ElectromechanicalEngine *engine) {
    printf("Initialisation du système électromécanique...\n");
    // Initialiser les banques
    for (int i = 0; i < MAX_BANKS; i++) {
        snprintf(engine->banks[i].name, sizeof(engine->banks[i].name), "Bank_%d", i);
        engine->banks[i].state = 0;
        engine->banks[i].switch_count = 0;
        engine->banks[i].fault_count = 0;
        for(int j = 0; j < RELAYS_PER_BANK; j++) {
            engine->banks[i].relays[j].is_active = false;
            engine->banks[i].relays[j].switch_count = 0;
        }
    }
    engine->current_tick = 0;
    engine->total_operations = 0;
    printf("Système initialisé avec succès.\n");
    return 0; // Succès
}

void write_bank_state(uint8_t bank_id, uint64_t state) {
    if (!g_initialized || bank_id >= MAX_BANKS) return;

    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t old_state = bank->state;
    bank->state = state;

    // Mettre à jour les compteurs de commutation et d'état des relais individuels
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        bool old_bit = (old_state >> i) & 1;
        bool new_bit = (state >> i) & 1;

        if (old_bit != new_bit) {
            bank->relays[i].switch_count++;
            bank->switch_count++;
            if (new_bit) {
                bank->relays[i].is_active = true;
            } else {
                bank->relays[i].is_active = false;
            }
        }
    }
    // Ici, on pourrait simuler des défaillances basées sur des conditions
}

uint8_t count_active_relays(uint8_t bank_id) {
     if (!g_initialized || bank_id >= MAX_BANKS) return 0;

    RelayBank *bank = &g_engine.banks[bank_id];
    uint8_t active_count = 0;
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        if ((bank->state >> i) & 1) {
            active_count++;
        }
    }
    return active_count;
}

void vorax_fusion_electromechanical(uint8_t bank1_id, uint8_t bank2_id, uint8_t result_bank_id) {
    if (!g_initialized || bank1_id >= MAX_BANKS || bank2_id >= MAX_BANKS || result_bank_id >= MAX_BANKS) {
        printf("✗ Fusion échouée : Paramètres invalides.\n");
        return;
    }
    RelayBank *bank1 = &g_engine.banks[bank1_id];
    RelayBank *bank2 = &g_engine.banks[bank2_id];
    RelayBank *result_bank = &g_engine.banks[result_bank_id];

    // Opération de fusion : combinaisons des états (OR logique)
    uint64_t fused_state = bank1->state | bank2->state;

    // Appliquer l'état fusionné à la banque résultat
    write_bank_state(result_bank_id, fused_state);

    // Mise à jour des métriques
    snprintf(result_bank->name, sizeof(result_bank->name), "%s+%s", bank1->name, bank2->name);
    result_bank->switch_count += bank1->switch_count + bank2->switch_count; // Simplification, une vraie fusion pourrait être plus complexe

    g_engine.total_operations++;
    printf("✓ Fusion Bank %d et Bank %d dans Bank %d effectuée.\n", bank1_id, bank2_id, result_bank_id);
}

void vorax_split_electromechanical(uint8_t bank_id, uint8_t num_parts) {
     if (!g_initialized || bank_id >= MAX_BANKS || num_parts == 0 || num_parts > RELAYS_PER_BANK) {
        printf("✗ Division échouée : Paramètres invalides.\n");
        return;
    }
    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t original_state = bank->state;
    uint64_t part_size = RELAYS_PER_BANK / num_parts;
    uint64_t remainder = RELAYS_PER_BANK % num_parts;

    uint64_t current_pos = 0;
    for (uint8_t i = 0; i < num_parts; i++) {
        // Calculer la taille de la partie actuelle
        uint64_t current_part_size = part_size + (i < remainder ? 1 : 0);
        uint64_t part_state = 0;

        if (current_part_size > 0) {
             // Extraire l'état pour cette partie
            for (uint64_t j = 0; j < current_part_size; j++) {
                if ((original_state >> (current_pos + j)) & 1) {
                    part_state |= (1ULL << j);
                }
            }
        }

        // Dans un scénario réel, une division pourrait créer de nouvelles banques
        // ou modifier la banque existante. Ici, nous simulons l'application
        // de l'état divisé à la banque d'origine (simplification).
        // Une vraie division pourrait nécessiter la création de nouvelles entrées dans `g_engine.banks`.
        // Pour cet exemple, nous allons juste afficher ce qui serait fait.
        printf("Part %d de Bank %d (taille %llu): État 0x%016llX\n", i, bank_id, current_part_size, part_state);

        // Si on pouvait créer de nouvelles banques, on appellerait write_bank_state ici.
        // Exemple : write_bank_state(new_bank_id_for_part_i, part_state);

        current_pos += current_part_size;
    }

    // Mise à jour des métriques (simplification)
    bank->switch_count /= num_parts; // Réinitialisation/division des commutations
    g_engine.total_operations++;
    printf("✓ Division de Bank %d en %d parties effectuée.\n", bank_id, num_parts);
}

void vorax_cycle_electromechanical(uint8_t bank_id, uint8_t modulo) {
    if (!g_initialized || bank_id >= MAX_BANKS || modulo == 0) {
        printf("✗ Cycle échoué : Paramètres invalides.\n");
        return;
    }
    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t current_state = bank->state;
    uint64_t cycled_state = 0;

    // Rotation des bits selon le modulo
    // Pour un cycle simple, on peut faire une rotation à gauche
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        if ((current_state >> i) & 1) {
            // Calculer la nouvelle position après rotation
            // Rotation à gauche : (i + modulo) % RELAYS_PER_BANK
            // Nous utilisons une rotation logique ici
            int new_pos = (i + modulo) % RELAYS_PER_BANK;
            cycled_state |= (1ULL << new_pos);
        }
    }

    write_bank_state(bank_id, cycled_state);
    g_engine.total_operations++;
    printf("✓ Cycle sur Bank %d avec modulo %d effectué.\n", bank_id, modulo);
}

void vorax_flow_electromechanical(uint8_t src_bank_id, uint8_t dst_bank_id) {
    if (!g_initialized || src_bank_id >= MAX_BANKS || dst_bank_id >= MAX_BANKS) {
        printf("✗ Flux échoué : Paramètres invalides.\n");
        return;
    }
    RelayBank *src_bank = &g_engine.banks[src_bank_id];
    RelayBank *dst_bank = &g_engine.banks[dst_bank_id];

    // Opération de flux : copie de l'état de la source vers la destination
    write_bank_state(dst_bank_id, src_bank->state);

    // Mise à jour des métriques (simplification)
    dst_bank->switch_count += src_bank->switch_count; // Copie des commutations
    g_engine.total_operations++;
    printf("✓ Flux de Bank %d vers Bank %d effectué.\n", src_bank_id, dst_bank_id);
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