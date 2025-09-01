
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
#include <sys/time.h>
#include <cjson/cJSON.h>
#include "lums/lums_backend.h"

// Configuration serveur
#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 8192
#define MAX_RESPONSE_SIZE 16384

// Structure client
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    pthread_t thread_id;
    time_t connection_time;
    uint64_t request_id;
} ClientConnection;

// Variables globales serveur
static int server_fd = -1;
static volatile int server_running = 0;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static ClientConnection active_clients[MAX_CLIENTS];
static int client_count = 0;
static uint64_t total_requests = 0;

// === GESTION SIGNAUX ===

void signal_handler(int signal) {
    printf("\n🛑 Signal %d reçu - Arrêt du serveur...\n", signal);
    server_running = 0;
    
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
    
    lums_backend_cleanup();
    exit(0);
}

// === UTILITAIRES HTTP ===

void send_http_response(int client_fd, int status_code, const char* content_type, const char* body) {
    char response[MAX_RESPONSE_SIZE];
    const char* status_text;
    
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    time_t now = time(NULL);
    struct tm* gmt = gmtime(&now);
    char date_str[100];
    strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    
    int content_length = body ? strlen(body) : 0;
    
    int header_len = snprintf(response, sizeof(response),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Date: %s\r\n"
        "Server: LUMS-HTTP/2.0\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, content_length, date_str
    );
    
    send(client_fd, response, header_len, 0);
    
    if (body && content_length > 0) {
        send(client_fd, body, content_length, 0);
    }
}

void send_json_response(int client_fd, int status_code, cJSON* json) {
    char* json_string = cJSON_Print(json);
    if (json_string) {
        send_http_response(client_fd, status_code, "application/json", json_string);
        free(json_string);
    } else {
        send_http_response(client_fd, 500, "application/json", "{\"error\":\"JSON serialization failed\"}");
    }
}

// === HANDLERS API ===

void handle_api_status(int client_fd, uint64_t request_id) {
    cJSON* response = cJSON_CreateObject();
    
    cJSON_AddStringToObject(response, "status", "operational");
    cJSON_AddStringToObject(response, "backend", "LUMS/VORAX");
    cJSON_AddStringToObject(response, "version", "2.1-scientific");
    cJSON_AddNumberToObject(response, "timestamp", time(NULL));
    cJSON_AddNumberToObject(response, "request_id", request_id);
    cJSON_AddNumberToObject(response, "total_requests", total_requests);
    cJSON_AddNumberToObject(response, "computations", lums_backend_get_total_computations());
    cJSON_AddNumberToObject(response, "energy_consumed", lums_backend_get_energy_consumed());
    cJSON_AddStringToObject(response, "backend_status", lums_backend_get_status());
    
    // Métriques système
    cJSON* metrics = cJSON_CreateObject();
    pthread_mutex_lock(&clients_mutex);
    cJSON_AddNumberToObject(metrics, "active_clients", client_count);
    pthread_mutex_unlock(&clients_mutex);
    cJSON_AddNumberToObject(metrics, "max_clients", MAX_CLIENTS);
    cJSON_AddItemToObject(response, "metrics", metrics);
    
    send_json_response(client_fd, 200, response);
    cJSON_Delete(response);
}

void handle_api_fusion(int client_fd, const char* body, uint64_t request_id) {
    cJSON* json = cJSON_Parse(body);
    if (!json) {
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    cJSON* lum_a_json = cJSON_GetObjectItem(json, "lum_a");
    cJSON* lum_b_json = cJSON_GetObjectItem(json, "lum_b");
    
    if (!cJSON_IsNumber(lum_a_json) || !cJSON_IsNumber(lum_b_json)) {
        cJSON_Delete(json);
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Missing or invalid lum_a/lum_b\"}");
        return;
    }
    
    uint64_t lum_a = (uint64_t)lum_a_json->valuedouble;
    uint64_t lum_b = (uint64_t)lum_b_json->valuedouble;
    uint64_t result;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    int status = lums_compute_fusion_real(lum_a, lum_b, &result);
    
    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) * 1000.0 + 
                           (end.tv_usec - start.tv_usec) / 1000.0;
    
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "request_id", request_id);
    
    if (status == 0) {
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddNumberToObject(response, "result", result);
        cJSON_AddNumberToObject(response, "result_hex", result);
        cJSON_AddNumberToObject(response, "input_a", lum_a);
        cJSON_AddNumberToObject(response, "input_b", lum_b);
        cJSON_AddNumberToObject(response, "lums_a", __builtin_popcountll(lum_a));
        cJSON_AddNumberToObject(response, "lums_b", __builtin_popcountll(lum_b));
        cJSON_AddNumberToObject(response, "lums_result", __builtin_popcountll(result));
        cJSON_AddNumberToObject(response, "execution_time_ms", execution_time);
        cJSON_AddBoolToObject(response, "conservation_valid", 
                             __builtin_popcountll(result) <= __builtin_popcountll(lum_a) + __builtin_popcountll(lum_b));
    } else {
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddNumberToObject(response, "error_code", status);
        cJSON_AddStringToObject(response, "error_message", 
                               status == -2 ? "Conservation violation" : "Invalid parameters");
    }
    
    send_json_response(client_fd, status == 0 ? 200 : 400, response);
    cJSON_Delete(response);
    cJSON_Delete(json);
}

void handle_api_sqrt(int client_fd, const char* body, uint64_t request_id) {
    cJSON* json = cJSON_Parse(body);
    if (!json) {
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    cJSON* input_json = cJSON_GetObjectItem(json, "input");
    if (!cJSON_IsNumber(input_json)) {
        cJSON_Delete(json);
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Missing or invalid input\"}");
        return;
    }
    
    double input = input_json->valuedouble;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    double result = lums_compute_sqrt_via_lums(input);
    
    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) * 1000.0 + 
                           (end.tv_usec - start.tv_usec) / 1000.0;
    
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "request_id", request_id);
    
    if (result >= 0) {
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddNumberToObject(response, "input", input);
        cJSON_AddNumberToObject(response, "result", result);
        cJSON_AddNumberToObject(response, "execution_time_ms", execution_time);
        cJSON_AddNumberToObject(response, "precision_error", fabs(result * result - input));
        cJSON_AddStringToObject(response, "method", "Newton-Raphson with LUM simulation");
    } else {
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "error_message", "Invalid input (negative number)");
    }
    
    send_json_response(client_fd, result >= 0 ? 200 : 400, response);
    cJSON_Delete(response);
    cJSON_Delete(json);
}

void handle_api_prime(int client_fd, const char* body, uint64_t request_id) {
    cJSON* json = cJSON_Parse(body);
    if (!json) {
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    cJSON* number_json = cJSON_GetObjectItem(json, "number");
    if (!cJSON_IsNumber(number_json)) {
        cJSON_Delete(json);
        send_http_response(client_fd, 400, "application/json", "{\"error\":\"Missing or invalid number\"}");
        return;
    }
    
    int number = (int)number_json->valuedouble;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    int is_prime = lums_test_prime_real(number);
    
    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) * 1000.0 + 
                           (end.tv_usec - start.tv_usec) / 1000.0;
    
    cJSON* response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "request_id", request_id);
    cJSON_AddNumberToObject(response, "number", number);
    cJSON_AddBoolToObject(response, "is_prime", is_prime == 1);
    cJSON_AddNumberToObject(response, "execution_time_ms", execution_time);
    cJSON_AddStringToObject(response, "method", "Trial division with LUM simulation");
    
    if (number >= 2) {
        cJSON_AddNumberToObject(response, "max_divisor_tested", (int)sqrt(number));
    }
    
    send_json_response(client_fd, 200, response);
    cJSON_Delete(response);
    cJSON_Delete(json);
}

// === ROUTEUR PRINCIPAL ===

void route_request(int client_fd, const char* method, const char* path, const char* body, uint64_t request_id) {
    printf("📥 [%lu] %s %s\n", request_id, method, path);
    
    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client_fd, 200, "text/plain", "");
        return;
    }
    
    if (strcmp(path, "/api/status") == 0 && strcmp(method, "GET") == 0) {
        handle_api_status(client_fd, request_id);
    } else if (strcmp(path, "/api/fusion") == 0 && strcmp(method, "POST") == 0) {
        handle_api_fusion(client_fd, body, request_id);
    } else if (strcmp(path, "/api/sqrt") == 0 && strcmp(method, "POST") == 0) {
        handle_api_sqrt(client_fd, body, request_id);
    } else if (strcmp(path, "/api/prime") == 0 && strcmp(method, "POST") == 0) {
        handle_api_prime(client_fd, body, request_id);
    } else {
        cJSON* error = cJSON_CreateObject();
        cJSON_AddStringToObject(error, "error", "Endpoint not found");
        cJSON_AddStringToObject(error, "method", method);
        cJSON_AddStringToObject(error, "path", path);
        cJSON_AddNumberToObject(error, "request_id", request_id);
        send_json_response(client_fd, 404, error);
        cJSON_Delete(error);
    }
}

// === HANDLER CLIENT ===

void* client_handler(void* arg) {
    ClientConnection* client = (ClientConnection*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    printf("🔗 Client connecté: %d\n", client->socket_fd);
    
    // Timeout socket
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(client->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    bytes_received = recv(client->socket_fd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        // Parse HTTP request
        char method[16], path[256], version[16];
        if (sscanf(buffer, "%15s %255s %15s", method, path, version) == 3) {
            
            // Chercher body (après double CRLF)
            char* body_start = strstr(buffer, "\r\n\r\n");
            char* body = body_start ? body_start + 4 : "";
            
            total_requests++;
            route_request(client->socket_fd, method, path, body, client->request_id);
        } else {
            send_http_response(client->socket_fd, 400, "text/plain", "Bad Request");
        }
    }
    
    close(client->socket_fd);
    
    // Retirer client de la liste
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (active_clients[i].socket_fd == client->socket_fd) {
            // Décaler les éléments
            for (int j = i; j < client_count - 1; j++) {
                active_clients[j] = active_clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    printf("🔌 Client déconnecté: %d\n", client->socket_fd);
    free(client);
    pthread_exit(NULL);
}

// === FONCTION PRINCIPALE ===

int main() {
    printf("🚀 Démarrage serveur HTTP LUMS/VORAX v2.1\n");
    
    // Configuration signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    // Initialisation backend LUMS
    if (lums_backend_init() != 0) {
        fprintf(stderr, "❌ Erreur initialisation backend LUMS\n");
        return 1;
    }
    
    // Test backend
    printf("🧪 Exécution tests backend...\n");
    if (lums_backend_comprehensive_test() != 0) {
        fprintf(stderr, "❌ Tests backend échoués\n");
        return 1;
    }
    
    // Création socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("❌ Erreur création socket");
        return 1;
    }
    
    // Configuration socket
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("❌ Erreur setsockopt");
        close(server_fd);
        return 1;
    }
    
    // Configuration adresse
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("❌ Erreur bind");
        close(server_fd);
        return 1;
    }
    
    // Listen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("❌ Erreur listen");
        close(server_fd);
        return 1;
    }
    
    printf("✅ Serveur HTTP démarré sur 0.0.0.0:%d\n", PORT);
    printf("📚 API disponible:\n");
    printf("   GET  /api/status  - État du système\n");
    printf("   POST /api/fusion  - Fusion de LUMs\n");
    printf("   POST /api/sqrt    - Calcul racine carrée\n");
    printf("   POST /api/prime   - Test de primalité\n");
    printf("🔬 Logs scientifiques: logs/scientific_traces/\n");
    printf("\n💡 Appuyez sur Ctrl+C pour arrêter\n\n");
    
    server_running = 1;
    uint64_t request_counter = 1;
    
    // Boucle principale
    while (server_running) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        
        if (client_fd < 0) {
            if (server_running) {
                perror("❌ Erreur accept");
            }
            continue;
        }
        
        // Vérifier limite clients
        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            send_http_response(client_fd, 503, "text/plain", "Server overloaded");
            close(client_fd);
            continue;
        }
        
        // Créer structure client
        ClientConnection* client = malloc(sizeof(ClientConnection));
        if (!client) {
            pthread_mutex_unlock(&clients_mutex);
            close(client_fd);
            continue;
        }
        
        client->socket_fd = client_fd;
        client->address = client_address;
        client->connection_time = time(NULL);
        client->request_id = request_counter++;
        
        // Ajouter à la liste
        active_clients[client_count] = *client;
        client_count++;
        
        pthread_mutex_unlock(&clients_mutex);
        
        // Créer thread client
        if (pthread_create(&client->thread_id, NULL, client_handler, client) != 0) {
            perror("❌ Erreur création thread");
            
            pthread_mutex_lock(&clients_mutex);
            client_count--;
            pthread_mutex_unlock(&clients_mutex);
            
            close(client_fd);
            free(client);
        } else {
            pthread_detach(client->thread_id);
        }
    }
    
    close(server_fd);
    lums_backend_cleanup();
    
    printf("🛑 Serveur arrêté proprement\n");
    return 0;
}
