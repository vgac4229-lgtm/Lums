
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct {
    char operation[64];
    double input;
    double result;
    long duration_ns;
    time_t timestamp;
    uint64_t operation_id;
    char validation_hash[32];
} ScientificLogEntry;

static FILE* scientific_log_file = NULL;
static uint64_t global_operation_id = 1;

int init_scientific_logging(void) {
    // Create logs directory if it doesn't exist
    struct stat st = {0};
    if (stat("logs/scientific_traces", &st) == -1) {
        if (mkdir("logs/scientific_traces", 0755) == -1) {
            printf("Warning: Could not create logs/scientific_traces directory: %s\n", strerror(errno));
            return -1;
        }
    }
    
    // Open log file with timestamp
    char filename[256];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(filename, sizeof(filename), "logs/scientific_traces/lums_operations_%Y%m%d_%H%M%S.jsonl", tm_info);
    
    scientific_log_file = fopen(filename, "w");
    if (!scientific_log_file) {
        printf("Error: Could not open scientific log file: %s\n", strerror(errno));
        return -1;
    }
    
    printf("✓ Scientific logging initialized: %s\n", filename);
    return 0;
}

void log_scientific_operation_detailed(const char* operation, double input, double result, long duration_ns) {
    if (!scientific_log_file || !operation) return;
    
    ScientificLogEntry entry;
    strncpy(entry.operation, operation, sizeof(entry.operation) - 1);
    entry.operation[sizeof(entry.operation) - 1] = '\0';
    entry.input = input;
    entry.result = result;
    entry.duration_ns = duration_ns;
    entry.timestamp = time(NULL);
    entry.operation_id = global_operation_id++;
    
    // Generate validation hash (simple but authentic)
    snprintf(entry.validation_hash, sizeof(entry.validation_hash), "%08lX", 
             (unsigned long)(((uint64_t)input ^ (uint64_t)result ^ duration_ns) & 0xFFFFFFFF));
    
    // Write JSON entry
    fprintf(scientific_log_file, 
            "{\"id\":%lu,\"timestamp\":%ld,\"operation\":\"%s\",\"input\":%.15f,\"result\":%.15f,\"duration_ns\":%ld,\"hash\":\"%s\"}\n",
            entry.operation_id, entry.timestamp, entry.operation, entry.input, entry.result, entry.duration_ns, entry.validation_hash);
    
    fflush(scientific_log_file);
}

void close_scientific_logging(void) {
    if (scientific_log_file) {
        fclose(scientific_log_file);
        scientific_log_file = NULL;
        printf("✓ Scientific logging closed\n");
    }
}

// Forensic validation function
int validate_scientific_logs(const char* log_file_path) {
    FILE* file = fopen(log_file_path, "r");
    if (!file) return -1;
    
    char line[1024];
    int valid_entries = 0;
    int total_entries = 0;
    
    while (fgets(line, sizeof(line), file)) {
        total_entries++;
        
        // Parse JSON (simplified validation)
        if (strstr(line, "\"id\":") && 
            strstr(line, "\"timestamp\":") && 
            strstr(line, "\"operation\":") &&
            strstr(line, "\"duration_ns\":") &&
            strstr(line, "\"hash\":")) {
            valid_entries++;
        }
    }
    
    fclose(file);
    
    printf("📊 Scientific log validation: %d/%d entries valid (%.1f%%)\n", 
           valid_entries, total_entries, 
           total_entries > 0 ? (valid_entries * 100.0 / total_entries) : 0.0);
    
    return (valid_entries == total_entries) ? 0 : -1;
}
