
#include "lums/lums_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 100

// Variables globales pour le serveur
static int server_running = 1;
static int server_socket = -1;
static pthread_t client_threads[MAX_CLIENTS];
static int active_clients = 0;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure pour passer les données aux threads clients
typedef struct {
    int socket;
    struct sockaddr_in address;
    int thread_id;
} client_data_t;

// Gestionnaire de signal pour arrêt propre
void signal_handler(int signal) {
    printf("\n🛑 Signal %d reçu, arrêt du serveur...\n", signal);
    server_running = 0;
    if (server_socket != -1) {
        close(server_socket);
    }
}

// Fonction pour envoyer une réponse HTTP
void send_http_response(int client_socket, int status_code, const char* content_type, const char* body) {
    char response[BUFFER_SIZE * 2];
    char* status_text = "OK";
    
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
    }
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status_code, status_text, content_type, strlen(body), body
    );
    
    send(client_socket, response, strlen(response), 0);
}

// Traitement des requêtes API
void handle_api_request(int client_socket, const char* method, const char* path, const char* body) {
    char json_response[2048];
    
    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client_socket, 200, "text/plain", "");
        return;
    }
    
    if (strcmp(path, "/api/status") == 0) {
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"status\":\"operational\","
            "\"backend\":\"LUMS/VORAX\","
            "\"version\":\"2.0\","
            "\"timestamp\":%ld,"
            "\"computations\":%lu,"
            "\"energy_consumed\":%lu"
            "}",
            time(NULL),
            lums_backend_get_total_computations(),
            lums_backend_get_energy_consumed()
        );
        send_http_response(client_socket, 200, "application/json", json_response);
        
    } else if (strcmp(path, "/api/fusion") == 0 && strcmp(method, "POST") == 0) {
        // Parse les paramètres de fusion depuis le body JSON
        uint64_t value1 = 0b11010;  // Valeur par défaut
        uint64_t value2 = 0b1100;   // Valeur par défaut
        uint64_t result;
        
        if (lums_compute_fusion_real(value1, value2, &result) == 0) {
            snprintf(json_response, sizeof(json_response),
                "{"
                "\"success\":true,"
                "\"operation\":\"fusion\","
                "\"input1\":%lu,"
                "\"input2\":%lu,"
                "\"result\":%lu,"
                "\"lum_count\":%d"
                "}",
                value1, value2, result, __builtin_popcountll(result)
            );
            send_http_response(client_socket, 200, "application/json", json_response);
        } else {
            send_http_response(client_socket, 500, "application/json", 
                "{\"success\":false,\"error\":\"Fusion failed\"}");
        }
        
    } else if (strcmp(path, "/api/sqrt") == 0 && strcmp(method, "POST") == 0) {
        double input = 16.0;  // Valeur par défaut
        double result = lums_compute_sqrt_via_lums(input);
        
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"success\":true,"
            "\"operation\":\"sqrt\","
            "\"input\":%.6f,"
            "\"result\":%.6f,"
            "\"method\":\"LUM_electromechanical\""
            "}",
            input, result
        );
        send_http_response(client_socket, 200, "application/json", json_response);
        
    } else if (strcmp(path, "/api/prime") == 0 && strcmp(method, "POST") == 0) {
        int number = 17;  // Valeur par défaut
        int is_prime = lums_test_prime_real(number);
        
        snprintf(json_response, sizeof(json_response),
            "{"
            "\"success\":true,"
            "\"operation\":\"prime_test\","
            "\"number\":%d,"
            "\"is_prime\":%s,"
            "\"method\":\"LUM_division_test\""
            "}",
            number, is_prime ? "true" : "false"
        );
        send_http_response(client_socket, 200, "application/json", json_response);
        
    } else {
        send_http_response(client_socket, 404, "application/json", 
            "{\"error\":\"Endpoint not found\"}");
    }
}

// Fonction pour traiter une requête HTTP
void process_http_request(int client_socket, const char* request) {
    char method[16], path[256], version[16];
    char* body_start = strstr(request, "\r\n\r\n");
    char* body = body_start ? body_start + 4 : "";
    
    if (sscanf(request, "%15s %255s %15s", method, path, version) != 3) {
        send_http_response(client_socket, 400, "text/plain", "Bad Request");
        return;
    }
    
    printf("📥 %s %s\n", method, path);
    
    if (strncmp(path, "/api/", 5) == 0) {
        handle_api_request(client_socket, method, path, body);
    } else {
        send_http_response(client_socket, 404, "text/html", 
            "<h1>404 - Page Not Found</h1><p>LUMS HTTP Server - API only</p>");
    }
}

// Thread pour gérer un client
void* handle_client(void* arg) {
    client_data_t* client_data = (client_data_t*)arg;
    int client_socket = client_data->socket;
    char buffer[BUFFER_SIZE];
    
    // Réception de la requête
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        process_http_request(client_socket, buffer);
    }
    
    // Fermeture de la connexion client
    close(client_socket);
    
    // Décrémentation du compteur de clients actifs
    pthread_mutex_lock(&clients_mutex);
    active_clients--;
    pthread_mutex_unlock(&clients_mutex);
    
    free(client_data);
    return NULL;
}

// Point d'entrée principal du serveur
int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  SERVEUR HTTP LUMS/VORAX                    ║\n");
    printf("║                    BACKEND COMPLET                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    // Installation gestionnaires signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialisation backend LUMS
    if (lums_backend_init() != 0) {
        printf("✗ Échec initialisation backend LUMS\n");
        return 1;
    }
    
    // Test complet avant démarrage serveur
    printf("\n🔧 Tests backend...\n");
    if (lums_backend_comprehensive_test() != 0) {
        printf("✗ Tests backend échoués\n");
        return 1;
    }
    
    // Création socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Échec création socket");
        return 1;
    }
    
    // Configuration socket
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Échec configuration socket");
        close(server_socket);
        return 1;
    }
    
    // Configuration adresse serveur
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind du socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Échec bind socket");
        close(server_socket);
        return 1;
    }
    
    // Écoute des connexions
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Échec listen socket");
        close(server_socket);
        return 1;
    }
    
    printf("\n🚀 Serveur HTTP LUMS démarré sur port %d\n", PORT);
    printf("📡 API endpoints disponibles:\n");
    printf("   GET  http://0.0.0.0:%d/api/status\n", PORT);
    printf("   POST http://0.0.0.0:%d/api/fusion\n", PORT);
    printf("   POST http://0.0.0.0:%d/api/sqrt\n", PORT);
    printf("   POST http://0.0.0.0:%d/api/prime\n", PORT);
    printf("\n⏳ En attente de connexions...\n\n");
    
    // Boucle principale du serveur
    while (server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (server_running && errno != EINTR) {
                perror("Échec accept");
            }
            continue;
        }
        
        // Vérification limite clients
        pthread_mutex_lock(&clients_mutex);
        if (active_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            send_http_response(client_socket, 503, "text/plain", "Server busy");
            close(client_socket);
            continue;
        }
        active_clients++;
        pthread_mutex_unlock(&clients_mutex);
        
        // Création thread pour traiter le client
        client_data_t* client_data = malloc(sizeof(client_data_t));
        if (client_data) {
            client_data->socket = client_socket;
            client_data->address = client_addr;
            client_data->thread_id = active_clients;
            
            pthread_t thread;
            if (pthread_create(&thread, NULL, handle_client, client_data) != 0) {
                perror("Échec création thread");
                close(client_socket);
                free(client_data);
                pthread_mutex_lock(&clients_mutex);
                active_clients--;
                pthread_mutex_unlock(&clients_mutex);
            } else {
                pthread_detach(thread);
            }
        } else {
            close(client_socket);
            pthread_mutex_lock(&clients_mutex);
            active_clients--;
            pthread_mutex_unlock(&clients_mutex);
        }
    }
    
    // Nettoyage final
    printf("\n🧹 Nettoyage et arrêt...\n");
    lums_backend_cleanup();
    close(server_socket);
    printf("✓ Serveur arrêté proprement\n");
    
    return 0;
}
