#ifndef ELECTROMECHANICAL_H
#define ELECTROMECHANICAL_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
    OPERATION_FUSION = 0x10,
    OPERATION_SPLIT = 0x11,
    OPERATION_CYCLE = 0x12,
    OPERATION_MEMORY = 0x13
} OperationType;

// Configuration système électromécanique
#define MAX_RELAYS              512     // 512 relais total
#define MAX_BANKS              8       // 8 banks de relais
#define RELAYS_PER_BANK        64      // 64 relais par bank
#define RELAY_SWITCHING_TIME_MS 8      // Temps commutation relais
#define RELAY_SETTLING_TIME_MS  2      // Temps stabilisation

// Types de relais
typedef enum {
    RELAY_OPEN = 0,
    RELAY_CLOSED = 1
} RelayState;

// Structure relais électromécanique
typedef struct {
    uint8_t bank_id;
    uint8_t relay_position;
    RelayState state;
    uint16_t voltage_mv;
    uint32_t switch_count;
    uint8_t is_faulty;
} ElectromechanicalRelay;

// Bank de relais
typedef struct {
    uint8_t bank_id;
    char name[32];
    uint64_t state;
    uint8_t faulty_relays[64];
    uint8_t fault_count;
    uint32_t switch_count;
    uint64_t last_switch_time;
} RelayBank;

// Moteur électromécanique
typedef struct {
    RelayBank banks[MAX_BANKS];
    uint8_t active_banks;
    uint64_t total_operations;
    uint64_t current_tick;
    char error_message[256];
} ElectromechanicalEngine;

// Variables globales - déclarations
// static variables defined in electromechanical.c

// Fonctions principales
int init_electromechanical_system(ElectromechanicalEngine *engine);
void set_relay_state(uint8_t bank, uint8_t position, RelayState state);
RelayState read_relay_state(uint8_t bank, uint8_t position);
void write_bank_state(uint8_t bank_id, uint64_t new_state);
uint64_t read_bank_state(uint8_t bank_id);

// Opérations VORAX électromécaniques
int vorax_fusion_electromechanical(uint8_t bank1, uint8_t bank2, uint8_t result_bank);
int vorax_split_electromechanical(uint8_t source_bank, uint8_t target_count);
int vorax_cycle_electromechanical(uint8_t bank, uint8_t modulo);
int vorax_flow_electromechanical(uint8_t source_bank, uint8_t target_bank);

// Utilitaires
uint64_t get_nanosecond_timestamp(void);
void delay_ms(uint32_t milliseconds);
uint8_t count_active_relays(uint8_t bank_id);
int verify_conservation(uint64_t before, uint64_t after);

void cleanup_electromechanical_system(ElectromechanicalEngine* engine);

// ElectromechanicalState management
ElectromechanicalState* create_electromechanical_state(void);
void destroy_electromechanical_state(ElectromechanicalState* state);
void simulate_relay_operation(ElectromechanicalState* state, uint64_t lum_a, uint64_t lum_b, OperationType op_type);

// Scientific logging
int init_scientific_logging(void);
void log_scientific_operation_detailed(const char* operation, double input, double result, long duration_ns);
void close_scientific_logging(void);
int validate_scientific_logs(const char* log_file_path);

#endif