#include <stdio.h>
#include <math.h>
#include <time.h>

int main() {
    printf("=== TEST CALCULS MATHÉMATIQUES RÉELS ===\n");
    
    // Test 1: Calcul de √2 par Newton-Raphson
    double x = 1.0;
    for (int i = 0; i < 10; i++) {
        x = (x + 2.0/x) / 2.0;
        printf("Itération %d: √2 ≈ %.15f\n", i+1, x);
    }
    
    // Test 2: Test de primalité pour 479
    int n = 479;
    int is_prime = 1;
    for (int i = 2; i*i <= n; i++) {
        if (n % i == 0) {
            is_prime = 0;
            break;
        }
    }
    printf("\nTest primalité %d: %s\n", n, is_prime ? "PREMIER" : "COMPOSÉ");
    
    // Test 3: Série Fibonacci
    printf("\nSérie Fibonacci:\n");
    long long a = 0, b = 1;
    for (int i = 0; i < 10; i++) {
        printf("F%d = %lld\n", i, a);
        long long temp = a + b;
        a = b;
        b = temp;
    }
    
    // Test 4: Timestamp réel
    time_t now = time(NULL);
    printf("\nTimestamp réel: %ld\n", now);
    
    return 0;
}
