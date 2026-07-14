#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define N 100000000L
#define SAVE_INTERVAL 500000L

int main() {
    clock_t t0 = clock();
    printf("LIQUVILLE L(N) ATE N = %ld\n", N);
    printf("========================================\n");
    printf("Alocando memoria (%ld MB)...\n", (N + 1) / (1024 * 1024));
    
    unsigned char *omega = (unsigned char*) calloc(N + 1, 1);
    if (!omega) { fprintf(stderr, "Falha na alocacao\n"); return 1; }
    
    printf("Crivo para omega(n)...\n");
    long p, n;
    for (p = 2; p <= N; p++) {
        if (omega[p] == 0) {  // p eh primo
            for (n = p; n <= N; n += p) {
                long temp = n;
                while (temp % p == 0) {
                    omega[n]++;
                    temp /= p;
                }
            }
        }
        if (p % 1000000 == 0) {
            printf("  processando primos ate %ld...\n", p);
        }
    }
    
    clock_t t1 = clock();
    double sieve_time = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("Crivo completo em %.1fs\n", sieve_time);
    
    printf("Calculando L(N)...\n");
    long L = 0;
    long max_abs = 0;
    long max_abs_N = 0;
    double max_ratio = 0.0;
    long max_ratio_N = 0;
    
    FILE *f = fopen("L_N_100M.txt", "w");
    if (!f) { fprintf(stderr, "Falha ao abrir arquivo\n"); return 1; }
    fprintf(f, "# N L(N) |L|/sqrt(N)\n");
    
    for (n = 1; n <= N; n++) {
        int lam = (omega[n] % 2 == 0) ? 1 : -1;
        L += lam;
        long absL = (L >= 0) ? L : -L;
        
        if (absL > max_abs) {
            max_abs = absL;
            max_abs_N = n;
        }
        
        if (n > 0) {
            double ratio = absL / sqrt((double)n);
            if (ratio > max_ratio) {
                max_ratio = ratio;
                max_ratio_N = n;
            }
        }
        
        if (n % SAVE_INTERVAL == 0 || n == N) {
            double ratio = (n > 0) ? absL / sqrt((double)n) : 0.0;
            fprintf(f, "%ld %ld %.6f\n", n, L, ratio);
        }
    }
    
    fclose(f);
    
    clock_t t2 = clock();
    double total_time = (double)(t2 - t0) / CLOCKS_PER_SEC;
    
    printf("\n========================================\n");
    printf("RESULTADOS\n");
    printf("========================================\n");
    printf("L(%ld) = %ld\n", N, L);
    printf("max |L(N)| = %ld  (em N = %ld)\n", max_abs, max_abs_N);
    printf("max |L|/sqrt(N) = %.4f  (em N = %ld)\n", max_ratio, max_ratio_N);
    printf("Tempo crivo: %.1fs\n", sieve_time);
    printf("Tempo total: %.1fs\n", total_time);
    
    // Distribuicao
    long count_m1 = 0, count_p1 = 0;
    for (n = 1; n <= N; n++) {
        if (omega[n] % 2 == 0) count_p1++;
        else count_m1++;
    }
    printf("\nDistribuicao de lambda(n):\n");
    printf("  +1 (par):   %ld (%.2f%%)\n", count_p1, 100.0 * count_p1 / N);
    printf("  -1 (impar): %ld (%.2f%%)\n", count_m1, 100.0 * count_m1 / N);
    
    printf("\nDados salvos em: L_N_100M.txt\n");
    
    free(omega);
    return 0;
}
