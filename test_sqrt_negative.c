
        #include "server/lums/lums.h"
        int main() {
          LUMGroup* result = sqrt_negative_via_lums(-16);

          printf("sqrt(-16) via LUMS: %zu inverted LUMs\n", result->count);
          printf("Inversion pattern: ");
          for(size_t i = 0; i < result->count && i < 10; i++) {
            printf("%d", result->lums[i].presence);
          }
          printf("\n");

          free_lum_group(result);
          return 0;
        }
      