
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include "lums_backend.h"

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10

static int server_socket = -1;
static int server_running = 1;

// Structure pour client thread
typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    int client_id;
} ClientContext;

// Gestionnaire signal propre
void signal_handler(int sig) {
    printf("\n🛑 Arrêt serveur LUMS (signal %d)\n", sig);
    server_running = 0;
    if (server_socket >= 0) {
        close(server_socket);
    }
    exit(0);
}

// Envoi réponse HTTP
void send_http_response(int client_socket, int status_code, const char* content_type, const char* body) {
    char response[BUFFER_SIZE];
    int body_length = body ? strlen(body) : 0;
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "\r\n"
        "%s",
        status_code,
        status_code == 200 ? "OK" : "Error",
        content_type,
        body_length,
        body ? body : ""
    );
    
    send(client_socket, response, strlen(response), 0);
}

// Traitement requête API LUMS
void handle_lums_api(int client_socket, const char* method, const char* path, const char* body) {
    char json_response[2048];
    
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/status") == 0) {
        // Status backend
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"status\":\"operational\","
            "\"backend_initialized\":true,"
            "\"total_computations\":%lu,"
            "\"energy_consumed\":%lu,"
            "\"computation_time_ms\":%.2f,"
            "\"timestamp\":%ld"
            "}",
            g_backend.total_computations,
            g_backend.energy_consumed,
            g_backend.computation_time_ms,
            time(NULL)
        );
        send_http_response(client_socket, 200, "application/json", json_response);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/fusion") == 0) {
        // Parse JSON body pour fusion
        uint64_t lum1 = 0b11010;  // Valeurs par défaut pour test
        uint64_t lum2 = 0b1100;
        
        // Parse basique du JSON (simplifié)
        if (body && strstr(body, "lum1")) {
            char* lum1_str = strstr(body, "\"lum1\":");
            if (lum1_str) {
                sscanf(lum1_str + 7, "%lu", &lum1);
            }
        }
        
        uint64_t result;
        int fusion_status = lums_compute_fusion_real(lum1, lum2, &result);
        
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"success\":%s,"
            "\"lum1\":%lu,"
            "\"lum2\":%lu,"
            "\"result\":%lu,"
            "\"lum_count\":%d,"
            "\"computation_time_ms\":%.2f"
            "}",
            fusion_status == 0 ? "true" : "false",
            lum1, lum2, result,
            __builtin_popcountll(result),
            g_backend.computation_time_ms
        );
        send_http_response(client_socket, 200, "application/json", json_response);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/prime") == 0) {
        // Test primalité
        uint64_t candidate = 17; // Défaut
        
        if (body && strstr(body, "number")) {
            char* num_str = strstr(body, "\"number\":");
            if (num_str) {
                sscanf(num_str + 9, "%lu", &candidate);
            }
        }
        
        int is_prime = lums_test_prime_real(candidate);
        
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"number\":%lu,"
            "\"is_prime\":%s,"
            "\"computation_time_ms\":%.2f,"
            "\"method\":\"electromechanical_lums\""
            "}",
            candidate,
            is_prime ? "true" : "false",
            g_backend.computation_time_ms
        );
        send_http_response(client_socket, 200, "application/json", json_response);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/sqrt") == 0) {
        // Calcul racine carrée
        double input = 16.0; // Défaut
        
        if (body && strstr(body, "value")) {
            char* val_str = strstr(body, "\"value\":");
            if (val_str) {
                sscanf(val_str + 8, "%lf", &input);
            }
        }
        
        double sqrt_result = lums_compute_sqrt_via_lums(input);
        
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"input\":%.6f,"
            "\"result\":%.10f,"
            "\"precision\":%.2e,"
            "\"computation_time_ms\":%.2f,"
            "\"method\":\"newton_raphson_lums\""
            "}",
            input, sqrt_result,
            fabs(sqrt_result * sqrt_result - input),
            g_backend.computation_time_ms
        );
        send_http_response(client_socket, 200, "application/json", json_response);
    }
    else {
        // API non trouvée
        send_http_response(client_socket, 404, "application/json", 
                          "{\"error\":\"API endpoint not found\"}");
    }
}

// Parse requête HTTP basique
void parse_http_request(const char* request, char* method, char* path, char* body) {
    // Extract method
    sscanf(request, "%s %s", method, path);
    
    // Extract body (après double CRLF)
    char* body_start = strstr(request, "\r\n\r\n");
    if (body_start) {
        strcpy(body, body_start + 4);
    } else {
        body[0] = '\0';
    }
}

// Thread client
void* handle_client_thread(void* arg) {
    ClientContext* ctx = (ClientContext*)arg;
    char buffer[BUFFER_SIZE];
    char method[16], path[256], body[1024];
    
    printf("📡 Client %d connecté\n", ctx->client_id);
    
    // Réception requête
    int bytes_received = recv(ctx->client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        // Parse requête
        parse_http_request(buffer, method, path, body);
        printf("📨 %s %s (client %d)\n", method, path, ctx->client_id);
        
        // Traitement API
        if (strncmp(path, "/api/", 5) == 0) {
            handle_lums_api(ctx->client_socket, method, path, body);
        } else {
            // Page d'accueil simple
            const char* html = 
                "<!DOCTYPE html>"
                "<html><head><title>LUMS Backend Server</title></head>"
                "<body>"
                "<h1>🧮 LUMS/VORAX Backend Server</h1>"
                "<p>Backend opérationnel avec calculs électromécaniques réels</p>"
                "<ul>"
                "<li><a href='/api/status'>Status Backend</a></li>"
                "<li>POST /api/fusion - Fusion LUMs</li>"
                "<li>POST /api/prime - Test primalité</li>"
                "<li>POST /api/sqrt - Racine carrée</li>"
                "</ul>"
                "</body></html>";
            send_http_response(ctx->client_socket, 200, "text/html", html);
        }
    }
    
    close(ctx->client_socket);
    printf("📴 Client %d déconnecté\n", ctx->client_id);
    free(ctx);
    return NULL;
}

// Serveur principal
int run_lums_http_server(void) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_counter = 0;
    
    // Création socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur création socket");
        return -1;
    }
    
    // Configuration socket
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(server_socket);
        return -2;
    }
    
    // Listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        close(server_socket);
        return -3;
    }
    
    printf("🚀 Serveur LUMS démarré sur port %d\n", PORT);
    printf("🌐 URL: http://0.0.0.0:%d/\n", PORT);
    
    // Boucle acceptation clients
    while (server_running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (server_running) {
                perror("Erreur accept");
            }
            continue;
        }
        
        // Création thread client
        ClientContext* ctx = malloc(sizeof(ClientContext));
        if (ctx) {
            ctx->client_socket = client_socket;
            ctx->client_addr = client_addr;
            ctx->client_id = ++client_counter;
            
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, handle_client_thread, ctx) != 0) {
                printf("✗ Erreur création thread client\n");
                close(client_socket);
                free(ctx);
            } else {
                pthread_detach(client_thread);
            }
        } else {
            close(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}

// Point d'entrée principal serveur
int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  SERVEUR HTTP LUMS/VORAX                    ║\n");
    printf("║                    BACKEND COMPLET                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    // Installation gestionnaires signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialisation backend
    if (lums_backend_init() != 0) {
        printf("✗ Échec initialisation backend LUMS\n");
        return 1;
    }
    
    // Test complet avant démarrage serveur
    printf("\n=== VALIDATION BACKEND AVANT DÉMARRAGE ===\n");
    if (lums_backend_comprehensive_test() != 0) {
        printf("✗ Échec validation backend\n");
        return 2;
    }
    
    // Démarrage serveur HTTP
    printf("\n=== DÉMARRAGE SERVEUR HTTP ===\n");
    return run_lums_http_server();
}
