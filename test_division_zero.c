
        #include "server/lums/lums.h"
        int main() {
          // Create test numerator
          LUM test_lums[4] = {{1, LUM_LINEAR, {0, 0}}, {0, LUM_LINEAR, {1, 0}}, 
                              {1, LUM_LINEAR, {2, 0}}, {1, LUM_LINEAR, {3, 0}}};
          LUMGroup* numerator = create_lum_group(test_lums, 4, GROUP_LINEAR);

          LUMGroup* result = resolve_division_by_zero(numerator);

          printf("Division by zero resolved: %zu fractal LUMs\n", result->count);
          printf("Pattern: ");
          for(size_t i = 0; i < result->count && i < 10; i++) {
            printf("%d", result->lums[i].presence);
          }
          printf("\n");

          free_lum_group(numerator);
          free_lum_group(result);
          return 0;
        }
      