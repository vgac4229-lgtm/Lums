
        #include "server/lums/lums.h"
        int main() {
          LUMGroup* result = represent_graham_number(10); // Precision level 10

          printf("Graham number (level 10): %zu expanded LUMs\n", result->count);
          printf("Knuth pattern: ");
          for(size_t i = 0; i < result->count && i < 20; i++) {
            printf("%d", result->lums[i].presence);
          }
          printf("\n");

          free_lum_group(result);
          return 0;
        }
      