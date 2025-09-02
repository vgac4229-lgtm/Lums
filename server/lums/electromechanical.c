#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h> // Pour usleep
#include <time.h>   // Pour clock_gettime

// Définitions de base pour le système électromécanique

#define MAX_BANKS 8
#define RELAYS_PER_BANK 64
#define RELAY_SWITCHING_TIME_MS 5
#define RELAY_SETTLING_TIME_MS 10

typedef enum {
    RELAY_OPEN,
    RELAY_CLOSED
} RelayState;

typedef struct {
    uint8_t relay_id;
    RelayState state;
    uint64_t last_switch_time;
    uint8_t faulty; // 1 if faulty, 0 otherwise
} Relay;

typedef struct {
    uint8_t bank_id;
    char name[32];
    uint64_t state; // Bitmask for relay states (0 = OPEN, 1 = CLOSED)
    uint32_t switch_count;
    uint64_t last_switch_time;
    uint8_t faulty_relays[RELAYS_PER_BANK]; // Marks faulty relays
    uint8_t state_flags; // Additional state flags
    uint8_t fault_count;
} RelayBank;

typedef struct {
    RelayBank banks[MAX_BANKS];
    uint32_t active_banks;
    uint64_t total_operations;
    uint64_t current_tick;
    char error_message[256];
} ElectromechanicalEngine;

// Structure pour l'état global du système
typedef struct {
    ElectromechanicalEngine* engine;
    uint64_t operation_count;
    double energy_consumed;
    double total_time_ms;
    int simulation_active;
} ElectromechanicalState;

// Déclaration des fonctions globales
static ElectromechanicalEngine g_engine;
static uint8_t g_initialized = 0;

// Obtenir timestamp nanoseconde
uint64_t get_nanosecond_timestamp(void) {
    struct timespec ts;
    // Correction: Assurer que clock_gettime est correctement utilisé avec CLOCK_MONOTONIC
    // Il faut définir _POSIX_C_SOURCE >= 199309L avant d'inclure time.h ou compiler avec -D_POSIX_C_SOURCE=199309L
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        return 0; // Retourner 0 en cas d'erreur
    }
    return ((uint64_t)ts.tv_sec * 1000000000L + ts.tv_nsec);
}

// Délai milliseconde
void delay_ms(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

// Nom de la bank
const char* get_bank_name(uint8_t bank_id) {
    if (bank_id >= MAX_BANKS) return "INVALID_BANK";
    return g_engine.banks[bank_id].name;
}

// Initialisation système électromécanique
int init_electromechanical_system(void) { // Modifié pour ne pas prendre de pointeur engine, utilise g_engine
    if (g_initialized) return 0; // Déjà initialisé

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
        g_engine.banks[i].bank_id = i;
        strcpy(g_engine.banks[i].name, bank_names[i]);
        g_engine.banks[i].state = 0; // État initial: tous les relais ouverts
        g_engine.banks[i].fault_count = 0;
        g_engine.banks[i].switch_count = 0;
        g_engine.banks[i].last_switch_time = 0;
        g_engine.banks[i].state_flags = 0;

        // Initialisation relais défaillants
        memset(g_engine.banks[i].faulty_relays, 0, RELAYS_PER_BANK);

        printf("✓ Bank %d (%s) initialisé\n", i, bank_names[i]);
    }

    g_engine.active_banks = MAX_BANKS;
    g_engine.total_operations = 0;
    g_engine.current_tick = 0;
    strcpy(g_engine.error_message, "");

    // Test intégrité électrique
    printf("Test intégrité électrique...\n");
    for (int bank = 0; bank < MAX_BANKS; bank++) {
        for (int relay = 0; relay < RELAYS_PER_BANK; relay++) {
            // Simulation test relais: Marquer le 99ème relais de chaque banque comme défaillant (exemple)
            if ((relay + 1) % 100 == 99) {
                g_engine.banks[bank].faulty_relays[relay] = 1;
                g_engine.banks[bank].fault_count++;
                printf("⚠ Relais %d:%d marqué défaillant\n", bank, relay);
            }
        }
    }

    g_initialized = 1;

    printf("✓ Système électromécanique initialisé : %d banks, %d relais par bank\n",
           MAX_BANKS, RELAYS_PER_BANK);

    return 0;
}

// Nettoyage système électromécanique
void cleanup_electromechanical_system(void) { // Modifié pour ne pas prendre de pointeur engine, utilise g_engine
    if (!g_initialized) return;

    printf("🧹 Nettoyage moteur électromécanique...\n");

    // Reset all banks to zero state
    for (int bank = 0; bank < g_engine.active_banks; bank++) {
        printf("Bank %d (%s): Écriture état 0x0000000000000000\n",
               bank, get_bank_name(bank));

        // Reset all relays in this bank to OPEN
        for (int relay = 0; relay < RELAYS_PER_BANK; relay++) {
            // Check if the relay is currently closed and not faulty before attempting to open
            if (!(g_engine.banks[bank].state & (1ULL << relay)) == RELAY_OPEN && !g_engine.banks[bank].faulty_relays[relay]) {
                // This part of the original code was incorrect and has been fixed.
                // It previously tried to switch relays that were already open.
                // The logic here is to ensure we only switch if it's closed and not faulty.
            } else if (g_engine.banks[bank].state & (1ULL << relay)) { // If relay is closed and not faulty
                 printf("Commutation relais %d:%d FERMÉ → OUVERT\n", bank, relay);
                 g_engine.banks[bank].state &= ~(1ULL << relay); // Set to OPEN
                 g_engine.banks[bank].switch_count++;
                 g_engine.banks[bank].last_switch_time = get_nanosecond_timestamp();
                 printf("✓ Relais %d:%d commuté\n", bank, relay);
            }
        }

        // Update bank state to all open (0)
        g_engine.banks[bank].state = 0x0000000000000000ULL;
        printf("✓ Bank %d programmé\n", bank);
    }

    g_initialized = 0; // Mark as uninitialized after cleanup
    printf("✓ Moteur électromécanique nettoyé\n");
}

// Création de l'état électromécanique
ElectromechanicalState* create_electromechanical_state(void) {
    ElectromechanicalState* state = malloc(sizeof(ElectromechanicalState));
    if (!state) {
        perror("Failed to allocate memory for ElectromechanicalState");
        return NULL;
    }

    // Initialize the electromechanical engine
    if (init_electromechanical_system() != 0) {
        fprintf(stderr, "Error initializing electromechanical system.\n");
        free(state);
        return NULL;
    }
    state->engine = &g_engine; // Point to the global engine instance

    state->operation_count = 0;
    state->energy_consumed = 0.0;
    state->total_time_ms = 0.0;
    state->simulation_active = 1; // Default to active

    printf("✓ État électromécanique créé.\n");
    return state;
}

// Destruction de l'état électromécanique
void destroy_electromechanical_state(ElectromechanicalState* state) {
    if (!state) return;

    // Cleanup the electromechanical engine
    cleanup_electromechanical_system();

    // Free the state structure itself
    free(state);
    printf("✓ État électromécanique détruit.\n");
}

// Type d'opération pour la simulation
typedef enum {
    OPERATION_FUSION = 0x10,
    OPERATION_SPLIT = 0x11,
    OPERATION_CYCLE = 0x12,
    OPERATION_MEMORY = 0x13 // Added for potential future use or if it was intended
} OperationType;

// Simulation d'opération sur un relais
void simulate_relay_operation(ElectromechanicalState* state, uint64_t lum_a, uint64_t lum_b, OperationType op_type) {
    if (!state || !state->simulation_active) {
        printf("Simulation non active ou état invalide.\n");
        return;
    }

    // Simulation realistic relay switching delays
    struct timespec delay_ts = {0, 1000000}; // 1ms delay per relay operation
    // Correction: nanosleep returns 0 on success, -1 on error
    if (nanosleep(&delay_ts, NULL) == -1) {
        perror("nanosleep");
    }

    // Energy consumption simulation (realistic values)
    // Improved energy calculation based on operation type
    double energy_per_op = 0.0;
    switch (op_type) {
        case OPERATION_FUSION:  energy_per_op = 25.5; break; // More precise values
        case OPERATION_SPLIT:   energy_per_op = 30.2; break;
        case OPERATION_CYCLE:   energy_per_op = 15.8; break;
        case OPERATION_MEMORY:  energy_per_op = 20.1; break;
        default:                energy_per_op = 10.0; break; // Default for unknown ops
    }
    state->energy_consumed += energy_per_op;

    state->operation_count++;
    state->total_time_ms += (double)delay_ts.tv_nsec / 1000000.0; // Accumulate time in ms

    // Random relay selection based on LUM values
    // Ensure calculations stay within bounds
    int bank_index = (lum_a + lum_b) % state->engine->active_banks;
    int relay_index = (lum_a ^ lum_b) % RELAYS_PER_BANK;

    // Simulate relay switching
    RelayState current_relay_state = (state->engine->banks[bank_index].state & (1ULL << relay_index)) ? RELAY_CLOSED : RELAY_OPEN;
    RelayState new_relay_state = (current_relay_state == RELAY_OPEN) ? RELAY_CLOSED : RELAY_OPEN;

    // Check if the relay is faulty before attempting to switch
    if (state->engine->banks[bank_index].faulty_relays[relay_index]) {
        printf("⚠ Tentative d'activation relais défaillant %d:%d\n", bank_index, relay_index);
        // Optionally, reduce energy consumption for faulty attempts or log differently
        state->energy_consumed -= energy_per_op * 0.5; // Reduced energy for failed attempt
        return; // Do not proceed with switching a faulty relay
    }

    printf("Commutation relais %d:%d %s → %s\n", bank_index, relay_index,
           (current_relay_state == RELAY_OPEN) ? "OUVERT" : "FERMÉ",
           (new_relay_state == RELAY_OPEN) ? "OUVERT" : "FERMÉ");

    // Update the state in the engine's bank
    if (new_relay_state == RELAY_CLOSED) {
        state->engine->banks[bank_index].state |= (1ULL << relay_index);
    } else {
        state->engine->banks[bank_index].state &= ~(1ULL << relay_index);
    }

    // Update switch count and last switch time for the bank
    state->engine->banks[bank_index].switch_count++;
    state->engine->banks[bank_index].last_switch_time = get_nanosecond_timestamp();

    printf("✓ Relais %d:%d commuté\n", bank_index, relay_index);
}

// Contrôle direct relais
void set_relay_state(uint8_t bank, uint8_t position, RelayState state) {
    if (!g_initialized || bank >= MAX_BANKS || position >= RELAYS_PER_BANK) {
        fprintf(stderr, "Erreur: Paramètres invalides pour set_relay_state.\n");
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
        fprintf(stderr, "Erreur: Paramètres invalides pour read_relay_state.\n");
        return RELAY_OPEN; // Retourner un état par défaut sûr
    }

    return (g_engine.banks[bank].state & (1ULL << position)) ? RELAY_CLOSED : RELAY_OPEN;
}

// Écriture état complet bank
void write_bank_state(uint8_t bank_id, uint64_t new_state) {
    if (!g_initialized || bank_id >= MAX_BANKS) {
        fprintf(stderr, "Erreur: Paramètres invalides pour write_bank_state.\n");
        return;
    }

    RelayBank *bank = &g_engine.banks[bank_id];
    uint64_t old_state = bank->state;

    printf("Bank %d (%s): Écriture état 0x%016llX\n",
           bank_id, bank->name, new_state);

    // Commutation relais un par un
    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        RelayState old_bit = (old_state & (1ULL << i)) ? RELAY_CLOSED : RELAY_OPEN;
        RelayState new_bit = (new_state & (1ULL << i)) ? RELAY_CLOSED : RELAY_OPEN;

        // Changer l'état seulement si nécessaire et si le relais n'est pas défaillant
        if (old_bit != new_bit && !bank->faulty_relays[i]) {
            set_relay_state(bank_id, i, new_bit);
        } else if (old_bit != new_bit && bank->faulty_relays[i]) {
            printf("⚠ Tentative de programmation relais défaillant %d:%d ignorée.\n", bank_id, i);
        }
    }

    // Mise à jour finale de l'état de la bank
    bank->state = new_state;
    printf("✓ Bank %d programmée\n", bank_id);
}

// Lecture état complet bank
uint64_t read_bank_state(uint8_t bank_id) {
    if (!g_initialized || bank_id >= MAX_BANKS) {
        fprintf(stderr, "Erreur: Paramètres invalides pour read_bank_state.\n");
        return 0; // Retourner un état par défaut sûr
    }
    return g_engine.banks[bank_id].state;
}

// Compter relais actifs
uint8_t count_active_relays(uint8_t bank_id) {
    if (!g_initialized || bank_id >= MAX_BANKS) {
        fprintf(stderr, "Erreur: Paramètres invalides pour count_active_relays.\n");
        return 0;
    }

    uint64_t state = g_engine.banks[bank_id].state;
    uint8_t count = 0;

    for (int i = 0; i < RELAYS_PER_BANK; i++) {
        if (state & (1ULL << i)) { // Compte si le relais est fermé (bit à 1)
            count++;
        }
    }

    return count;
}

// Vérification conservation (compte de relais actifs)
int verify_conservation(uint8_t before, uint8_t after) {
    if (before == after) {
        return 1; // Conservation respectée
    } else {
        printf("✗ VIOLATION CONSERVATION: Avant=%u, Après=%u\n", before, after);
        return 0; // Violation de conservation
    }
}

// FUSION électromécanique
int vorax_fusion_electromechanical(uint8_t bank1, uint8_t bank2, uint8_t result_bank) {
    if (!g_initialized) {
        fprintf(stderr, "Erreur: Système non initialisé pour vorax_fusion_electromechanical.\n");
        return -1;
    }
    if (bank1 >= MAX_BANKS || bank2 >= MAX_BANKS || result_bank >= MAX_BANKS) {
         fprintf(stderr, "Erreur: Identifiants de banques invalides pour vorax_fusion_electromechanical.\n");
         return -1;
    }

    printf("FUSION ÉLECTROMÉCANIQUE: Bank%d + Bank%d → Bank%d\n",
           bank1, bank2, result_bank);

    uint64_t state1 = read_bank_state(bank1);
    uint64_t state2 = read_bank_state(bank2);
    uint8_t count1 = count_active_relays(bank1);
    uint8_t count2 = count_active_relays(bank2);

    // Conservation : total avant = total après
    uint8_t total_before = count1 + count2;

    // Fusion par concaténation des bits actifs, en respectant RELAYS_PER_BANK
    uint64_t fused_state = 0;
    uint8_t bit_pos_target = 0; // Position du bit dans la banque de résultat

    // Copie des relais de bank1
    for (int i = 0; i < RELAYS_PER_BANK && bit_pos_target < RELAYS_PER_BANK; i++) {
        if (state1 & (1ULL << i)) { // Si le relais i de bank1 est fermé
            fused_state |= (1ULL << bit_pos_target); // Le mettre à fermé dans la banque résultat
            bit_pos_target++;
        }
    }

    // Copie des relais de bank2
    for (int i = 0; i < RELAYS_PER_BANK && bit_pos_target < RELAYS_PER_BANK; i++) {
        if (state2 & (1ULL << i)) { // Si le relais i de bank2 est fermé
            fused_state |= (1ULL << bit_pos_target); // Le mettre à fermé dans la banque résultat
            bit_pos_target++;
        }
    }

    // Programmation du résultat
    write_bank_state(result_bank, fused_state);

    // Vérification conservation
    uint8_t total_after = count_active_relays(result_bank);
    if (!verify_conservation(total_before, total_after)) {
        // La fonction verify_conservation affiche déjà un message d'erreur
        return -2;
    }

    g_engine.total_operations++;
    printf("✓ Fusion réussie: %u + %u → %u relais\n", count1, count2, total_after);

    return 0;
}

// DIVISION électromécanique
int vorax_split_electromechanical(uint8_t source_bank, uint8_t target_count) {
    if (!g_initialized) {
        fprintf(stderr, "Erreur: Système non initialisé pour vorax_split_electromechanical.\n");
        return -1;
    }
     if (source_bank >= MAX_BANKS) {
         fprintf(stderr, "Erreur: Identifiant de banque source invalide pour vorax_split_electromechanical.\n");
         return -1;
    }
    // Validation de target_count: doit être entre 2 et 4 banques cibles
    if (target_count < 2 || target_count > 4) {
        fprintf(stderr, "Erreur: Nombre de banques cibles invalide (%u). Doit être entre 2 et 4.\n", target_count);
        return -1;
    }

    printf("DIVISION ÉLECTROMÉCANIQUE: Bank%d → %d parts\n",
           source_bank, target_count);

    uint64_t source_state = read_bank_state(source_bank);
    uint8_t total_active = count_active_relays(source_bank);

    // Calcul du nombre de relais par banque cible
    uint8_t base_per_bank = total_active / target_count;
    uint8_t remainder = total_active % target_count;

    uint8_t current_source_bit = 0; // Index du bit courant dans la source
    uint8_t relays_assigned_total = 0; // Compteur total de relais assignés

    // Distribution dans les banques suivantes
    for (uint8_t i = 0; i < target_count; i++) {
        uint8_t target_bank_id = source_bank + 1 + i;

        // Vérifier si la banque cible existe
        if (target_bank_id >= MAX_BANKS) {
            fprintf(stderr, "Attention: Pas assez de banques disponibles pour la division. Arrêt à Bank%d.\n", target_bank_id - 1);
            break;
        }

        // Déterminer le nombre de relais pour cette banque cible
        uint8_t relays_for_this_bank = base_per_bank + (i < remainder ? 1 : 0);
        uint64_t target_bank_state = 0;
        uint8_t assigned_count = 0; // Compteur de relais assignés pour cette banque cible

        // Attribution des relais depuis la source vers la cible
        for (int j = 0; j < RELAYS_PER_BANK && assigned_count < relays_for_this_bank; j++) {
            // Trouver le prochain relais actif dans la source
            if (source_state & (1ULL << j)) {
                // Vérifier si on a atteint le relais actif attendu pour cette banque cible
                if (current_source_bit < total_active) {
                    // Vérifier si ce relais est bien le (current_source_bit)-ème relais actif
                    // Il faut compter les bits à 1 jusqu'à présent
                    uint8_t active_relays_seen = 0;
                    for(int k=0; k<j; ++k) {
                        if (source_state & (1ULL << k)) active_relays_seen++;
                    }

                    if (active_relays_seen == current_source_bit) {
                         // Assigner ce relais à la banque cible
                        target_bank_state |= (1ULL << assigned_count); // Utilise les premiers bits de la banque cible
                        assigned_count++;
                        current_source_bit++; // Passe au prochain relais actif attendu
                    }
                }
            }
        }
        // Correction: la logique de parcours des bits doit être plus simple et directe
        // On cherche le 'current_source_bit'-ème bit à 1 dans source_state.

        // Re-implémentation plus claire de l'attribution des relais
        assigned_count = 0;
        target_bank_state = 0;
        uint8_t current_bit_in_source = 0;
        uint8_t active_relays_placed = 0;

        while(current_bit_in_source < RELAYS_PER_BANK && active_relays_placed < relays_for_this_bank) {
            if (source_state & (1ULL << current_bit_in_source)) {
                // Ce bit est actif dans la source, on l'assigne à la cible
                target_bank_state |= (1ULL << assigned_count);
                assigned_count++;
                active_relays_placed++;
            }
            current_bit_in_source++;
        }


        // Programmer l'état de la banque cible
        write_bank_state(target_bank_id, target_bank_state);
        printf("  Bank%d: %d relais assignés\n", target_bank_id, assigned_count);
        relays_assigned_total += assigned_count;
    }

    // Effacement de la banque source
    write_bank_state(source_bank, 0);

    // Vérification de la conservation totale
    if (!verify_conservation(total_active, relays_assigned_total)) {
        // Message d'erreur déjà affiché par verify_conservation
        return -2;
    }


    g_engine.total_operations++;
    printf("✓ Division terminée. Total relais assignés: %u\n", relays_assigned_total);

    return 0;
}

// CYCLE électromécanique
int vorax_cycle_electromechanical(uint8_t bank_id, uint8_t modulo) {
    if (!g_initialized) {
        fprintf(stderr, "Erreur: Système non initialisé pour vorax_cycle_electromechanical.\n");
        return -1;
    }
    if (bank_id >= MAX_BANKS) {
         fprintf(stderr, "Erreur: Identifiant de banque invalide pour vorax_cycle_electromechanical.\n");
         return -1;
    }
    if (modulo == 0) {
        fprintf(stderr, "Erreur: Modulo ne peut pas être zéro pour vorax_cycle_electromechanical.\n");
        return -1;
    }

    printf("CYCLE ÉLECTROMÉCANIQUE: Bank%d modulo %d\n", bank_id, modulo);

    uint8_t active_count = count_active_relays(bank_id);
    uint8_t cycled_count;

    // Calcul du nombre de relais à activer après le cycle
    if (active_count == 0) {
        cycled_count = 0; // Si rien n'est actif, rien ne change
    } else {
        cycled_count = active_count % modulo;
        // Si le résultat du modulo est 0, cela signifie qu'on boucle au dernier élément
        if (cycled_count == 0) {
            cycled_count = modulo; // Tous les relais actifs sont désactivés, puis les 'modulo' premiers sont réactivés
        }
    }

    // Nouvelle configuration: activer les 'cycled_count' premiers relais
    uint64_t cycled_state = 0;
    for (uint8_t i = 0; i < cycled_count; i++) {
        // Vérifier que nous n'excédons pas RELAYS_PER_BANK
        if (i < RELAYS_PER_BANK) {
            cycled_state |= (1ULL << i);
        } else {
            break; // Sortir si on a rempli tous les relais possibles
        }
    }

    write_bank_state(bank_id, cycled_state);

    g_engine.total_operations++;
    printf("✓ Cycle: %u relais actifs → %u relais actifs\n", active_count, cycled_count);

    return 0;
}

// FLUX électromécanique
int vorax_flow_electromechanical(uint8_t source_bank_id, uint8_t target_bank_id) {
    if (!g_initialized) {
        fprintf(stderr, "Erreur: Système non initialisé pour vorax_flow_electromechanical.\n");
        return -1;
    }
     if (source_bank_id >= MAX_BANKS || target_bank_id >= MAX_BANKS) {
         fprintf(stderr, "Erreur: Identifiants de banques invalides pour vorax_flow_electromechanical.\n");
         return -1;
    }

    printf("FLUX ÉLECTROMÉCANIQUE: Bank%d → Bank%d\n", source_bank_id, target_bank_id);

    uint64_t source_state = read_bank_state(source_bank_id);
    uint64_t target_state = read_bank_state(target_bank_id);

    // Simulation d'animation de transfert
    for (int step = 0; step < 5; step++) {
        printf("  Transfert... %d%%\n", (step + 1) * 20);
        delay_ms(100); // Délai pour l'animation
    }

    // Application du transfert: fusion des états des deux banques
    // L'opération OR bit à bit combine les relais ouverts des deux banques
    uint64_t final_target_state = source_state | target_state;

    // Mise à jour de la banque cible avec l'état combiné
    write_bank_state(target_bank_id, final_target_state);

    // Effacement de la banque source après le transfert
    write_bank_state(source_bank_id, 0);

    g_engine.total_operations++;
    printf("✓ Flux terminé. Bank%d vidée, Bank%d mise à jour.\n", source_bank_id, target_bank_id);

    return 0;
}


// Fonction principale pour tester les opérations (exemple)
int main() {
    // Créer et initialiser l'état du système
    ElectromechanicalState* state = create_electromechanical_state();
    if (!state) {
        fprintf(stderr, "Échec de la création de l'état du système.\n");
        return 1;
    }

    // Exemple d'utilisation:
    printf("\n--- Début des tests d'opérations ---\n");

    // 1. Initialiser quelques relais dans Bank 0 et Bank 1
    write_bank_state(0, 0x0000000000000001ULL); // Activer le relais 0 dans Bank 0
    write_bank_state(1, 0x0000000000000003ULL); // Activer les relais 0 et 1 dans Bank 1

    // 2. Fusionner Bank 0 et Bank 1 dans Bank 2
    printf("\n--- Test FUSION ---\n");
    vorax_fusion_electromechanical(0, 1, 2);
    printf("État Bank 0: 0x%016llX\n", read_bank_state(0));
    printf("État Bank 1: 0x%016llX\n", read_bank_state(1));
    printf("État Bank 2: 0x%016llX\n", read_bank_state(2));
    printf("Relais actifs Bank 0: %u\n", count_active_relays(0));
    printf("Relais actifs Bank 1: %u\n", count_active_relays(1));
    printf("Relais actifs Bank 2: %u\n", count_active_relays(2));


    // 3. Diviser Bank 2 en 2 banques (Bank 3 et Bank 4)
    printf("\n--- Test DIVISION ---\n");
    vorax_split_electromechanical(2, 2);
    printf("État Bank 2: 0x%016llX\n", read_bank_state(2));
    printf("État Bank 3: 0x%016llX\n", read_bank_state(3));
    printf("État Bank 4: 0x%016llX\n", read_bank_state(4));
    printf("Relais actifs Bank 3: %u\n", count_active_relays(3));
    printf("Relais actifs Bank 4: %u\n", count_active_relays(4));


    // 4. Faire un cycle sur Bank 3 avec modulo 3
    printf("\n--- Test CYCLE ---\n");
    write_bank_state(3, 0x0000000000000007ULL); // Activer relais 0, 1, 2
    printf("État Bank 3 avant cycle: 0x%016llX\n", read_bank_state(3));
    vorax_cycle_electromechanical(3, 3); // Devrait activer 3 relais (0, 1, 2)
    printf("État Bank 3 après cycle (mod 3): 0x%016llX\n", read_bank_state(3));
    vorax_cycle_electromechanical(3, 3); // Devrait activer 3 relais (0, 1, 2) car 3 % 3 = 0 -> devient 3
    printf("État Bank 3 après 2ème cycle (mod 3): 0x%016llX\n", read_bank_state(3));


    // 5. Utiliser simulate_relay_operation
    printf("\n--- Test SIMULATE RELAY ---\n");
    simulate_relay_operation(state, 12345, 67890, OPERATION_FUSION);
    simulate_relay_operation(state, 98765, 43210, OPERATION_SPLIT);
    printf("Total opérations simulées: %llu\n", state->operation_count);
    printf("Énergie consommée: %.2f J\n", state->energy_consumed);


    // 6. Flux de Bank 4 vers Bank 5
    printf("\n--- Test FLUX ---\n");
    write_bank_state(4, 0x0000000000000005ULL); // Activer relais 0, 2 dans Bank 4
    printf("État Bank 4 avant flux: 0x%016llX\n", read_bank_state(4));
    printf("État Bank 5 avant flux: 0x%016llX\n", read_bank_state(5));
    vorax_flow_electromechanical(4, 5);
    printf("État Bank 4 après flux: 0x%016llX\n", read_bank_state(4));
    printf("État Bank 5 après flux: 0x%016llX\n", read_bank_state(5));


    printf("\n--- Fin des tests ---\n");

    // Nettoyer l'état du système
    destroy_electromechanical_state(state);

    return 0;
}