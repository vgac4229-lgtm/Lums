
        #include "server/lums/lums.h"
        int main() {
          // Test critical line s = 0.5 + it
          int result = test_riemann_hypothesis_lums(0.5, 14.134725); // First non-trivial zero

          printf("Riemann hypothesis test (s=0.5+14.134725i): %s\n", 
                 result ? "RESONANCE DETECTED" : "NO RESONANCE");

          return 0;
        }
      