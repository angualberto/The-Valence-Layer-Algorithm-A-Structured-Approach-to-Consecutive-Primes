/*
 * Algoritmo da Camada de Valencia - Implementacao C + GMP
 * Encontra o proximo primo apos N e decompoe o gap
 * Compilar:
 *   gcc -O3 -march=native valencia_gmp.c -o valencia_gmp -lgmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

/* Caixa de ferramentas de valencia (otimizada para gaps grandes) */
static const long FERRAMENTAS[] = {1000, 500, 200, 120, 64, 34,
                                   32, 30, 28, 24, 20, 18, 16, 12,
                                   10, 8, 6, 4, 2};
static const int N_FERRAMENTAS = 19;

/* Decompoe o gap (guloso: maiores primeiro, quantas vezes necessario) */
void decompor_gap(mpz_t gap, long *contagens) {
    mpz_t resto, q, f;
    mpz_inits(resto, q, f, NULL);
    mpz_set(resto, gap);
    
    for (int i = 0; i < N_FERRAMENTAS; i++) {
        contagens[i] = 0;
        mpz_set_si(f, FERRAMENTAS[i]);
        if (mpz_cmp(resto, f) >= 0) {
            mpz_tdiv_q(q, resto, f);
            contagens[i] = mpz_get_si(q);
            mpz_submul(resto, f, q);
        }
    }
    
    mpz_clears(resto, q, f, NULL);
}

/* Decompoe variado (cada ferramenta no maximo 1x, resto com 2) */
void decompor_gap_variado(mpz_t gap, long *contagens) {
    mpz_t resto, f;
    mpz_inits(resto, f, NULL);
    mpz_set(resto, gap);
    
    for (int i = 0; i < N_FERRAMENTAS; i++) {
        contagens[i] = 0;
    }
    
    for (int i = 0; i < N_FERRAMENTAS - 1; i++) {
        mpz_set_si(f, FERRAMENTAS[i]);
        if (mpz_cmp(resto, f) >= 0) {
            contagens[i] = 1;
            mpz_sub(resto, resto, f);
        }
    }
    
    /* Resto com 2 */
    if (mpz_divisible_ui_p(resto, 2)) {
        mpz_divexact_ui(resto, resto, 2);
        contagens[N_FERRAMENTAS - 1] = mpz_get_si(resto);
    }
    
    mpz_clears(resto, f, NULL);
}

/* Encontra o proximo primo >= start com Miller-Rabin (GMP nativo) */
void proximo_primo(mpz_t result, mpz_t start, int repeticoes) {
    if (mpz_cmp_ui(start, 2) < 0) {
        mpz_set_ui(result, 2);
        return;
    }
    if (mpz_cmp_ui(start, 2) == 0) {
        mpz_set_ui(result, 3);
        return;
    }
    
    /* Proximo impar apos start */
    mpz_set(result, start);
    mpz_add_ui(result, result, 1);
    if (mpz_even_p(result)) {
        mpz_add_ui(result, result, 1);
    }
    
    /* Testa candidatos ate achar primo */
    while (1) {
        int r = mpz_probab_prime_p(result, repeticoes);
        if (r) {
            return;
        }
        mpz_add_ui(result, result, 2);
    }
}

/* Encontra o antecessor primo (maior primo < start) */
void antecessor_primo(mpz_t result, mpz_t start) {
    if (mpz_cmp_ui(start, 3) <= 0) {
        mpz_set_ui(result, 2);
        return;
    }
    
    mpz_set(result, start);
    mpz_sub_ui(result, result, 1);
    
    if (mpz_even_p(result)) {
        mpz_sub_ui(result, result, 1);
    }
    
    while (1) {
        int r = mpz_probab_prime_p(result, 25);
        if (r == 1 || r == 2) {
            return;
        }
        mpz_sub_ui(result, result, 2);
    }
}

void imprimir_decomposicao(long *contagens) {
    int primeiro = 1;
    for (int i = 0; i < N_FERRAMENTAS; i++) {
        if (contagens[i] > 0) {
            if (!primeiro) printf(" + ");
            printf("%ldx%ld", contagens[i], FERRAMENTAS[i]);
            primeiro = 0;
        }
    }
    if (primeiro) printf("(vazio)");
    printf("\n");
}

void benchmark_aleatorio(int digitos) {
    mpz_t N, primo, gap;
    long contagens[N_FERRAMENTAS];
    clock_t t0, t1;
    unsigned long seed;
    FILE *urandom;
    
    mpz_inits(N, primo, gap, NULL);
    
    /* Gera numero aleatorio impar com 'digitos' digitos */
    urandom = fopen("/dev/urandom", "rb");
    if (urandom) {
        fread(&seed, sizeof(seed), 1, urandom);
        fclose(urandom);
    } else {
        seed = time(NULL);
    }
    srand(seed);
    
    /* Gera no formato de string decimal */
    char *buf = malloc(digitos + 2);
    buf[0] = '1' + (rand() % 9);  /* primeiro digito nao zero */
    for (int i = 1; i < digitos; i++) {
        buf[i] = '0' + (rand() % 10);
    }
    buf[digitos] = '\0';
    mpz_set_str(N, buf, 10);
    free(buf);
    
    /* Garante impar */
    if (mpz_even_p(N)) mpz_add_ui(N, N, 1);
    
    printf("\n--- Benchmark: %d digitos ---\n", digitos);
    printf("N = %s...%s\n",
           mpz_get_str(NULL, 10, N) + 0,
           mpz_get_str(NULL, 10, N) + digitos - 20);
    
    t0 = clock();
    proximo_primo(primo, N, 25);
    t1 = clock();
    mpz_sub(gap, primo, N);
    
    {
        char *gs = mpz_get_str(NULL, 10, gap);
        printf("Gap: %s  (%.3fs)\n", gs, (double)(t1-t0)/CLOCKS_PER_SEC);
        free(gs);
    }
    
    if (mpz_even_p(gap)) {
        decompor_gap(gap, contagens);
        printf("Decomp (gulosa): ");
        imprimir_decomposicao(contagens);
    }
    
    mpz_clears(N, primo, gap, NULL);
}

void teste_numero(const char *str_n, const char *label) {
    mpz_t N, primo, gap;
    long contagens[N_FERRAMENTAS];
    clock_t t0, t1;
    
    mpz_inits(N, primo, gap, NULL);
    mpz_set_str(N, str_n, 10);
    
    printf("\n--- %s ---\n", label);
    {
        char *s = mpz_get_str(NULL, 10, N);
        printf("N: %s...%s  (%d dig)\n", s, s + strlen(s) - 20, (int)strlen(s));
        free(s);
    }
    
    t0 = clock();
    int r = mpz_probab_prime_p(N, 25);
    t1 = clock();
    printf("Primo? %s (%.3fs)\n",
           r ? "SIM" : "NAO", (double)(t1-t0)/CLOCKS_PER_SEC);
    
    if (r) {
        t0 = clock();
        proximo_primo(primo, N, 25);
        t1 = clock();
        mpz_sub(gap, primo, N);
        {
            char *gs = mpz_get_str(NULL, 10, gap);
            printf("Prox primo gap: %s (%.3fs)\n", gs, (double)(t1-t0)/CLOCKS_PER_SEC);
            if (mpz_even_p(gap)) {
                decompor_gap(gap, contagens);
                printf("Decomp: "); imprimir_decomposicao(contagens);
            }
            free(gs);
        }
    }
    
    mpz_clears(N, primo, gap, NULL);
}

int main(int argc, char **argv) {
    printf("============================================\n");
    printf("ALGORITMO DA CAMADA DE VALENCIA - C + GMP\n");
    printf("============================================\n");
    printf("Ferramentas: ");
    for (int i = 0; i < N_FERRAMENTAS; i++) {
        printf("%ld%s", FERRAMENTAS[i], i < N_FERRAMENTAS-1 ? ", " : "\n");
    }
    printf("\n");
    
    if (argc >= 2 && strcmp(argv[1], "--cauda") == 0) {
        /* Modo analise de cauda (ultimos digitos) */
        int qtd = 1000;
        char *start_str = NULL;
        if (argc >= 3) qtd = atoi(argv[2]);
        if (argc >= 4) start_str = argv[3];
        
        mpz_t atual, primo, gap;
        mpz_inits(atual, primo, gap, NULL);
        
        if (start_str) {
            mpz_set_str(atual, start_str, 10);
        } else {
            /* Default: 10^100 */
            mpz_ui_pow_ui(atual, 10, 100);
        }
        
        printf("Gerando %d primos a partir de ", qtd);
        {
            char *s = mpz_get_str(NULL, 10, atual);
            int len = strlen(s);
            if (len > 40) printf("%.20s...%.20s (%d dig)\n", s, s+len-20, len);
            else printf("%s (%d dig)\n", s, len);
            free(s);
        }
        
        int count[10] = {0};
        long total_gap = 0;
        clock_t t0 = clock();
        
        for (int i = 0; i < qtd; i++) {
            proximo_primo(primo, atual, 25);
            mpz_sub(gap, primo, atual);
            total_gap += mpz_get_si(gap);
            
            int last_digit = mpz_fdiv_ui(primo, 10);
            count[last_digit]++;
            
            mpz_set(atual, primo);
            mpz_add_ui(atual, atual, 1);
            
            if ((i+1) % 200 == 0) {
                printf("  %d/%d primos...\n", i+1, qtd);
            }
        }
        clock_t t1 = clock();
        
        printf("\n=== ANALISE DA CAUDA ===\n");
        printf("Primos: %d\n", qtd);
        printf("Tempo: %.2fs\n", (double)(t1-t0)/CLOCKS_PER_SEC);
        printf("Gap medio: %.1f\n", (double)total_gap/qtd);
        printf("\n%8s | %10s | %10s | %10s\n", "Digito", "Contagem", "Proporcao", "Esperado");
        printf("--------+------------+------------+------------\n");
        int digitos_lista[] = {1, 3, 7, 9};
        for (int di = 0; di < 4; di++) {
            int d = digitos_lista[di];
            double prop = 100.0 * count[d] / qtd;
            printf("%8d | %10d | %8.2f%% | %10s\n", d, count[d], prop, "25.00%");
        }
        int outros = 0;
        for (int d = 0; d < 10; d++) {
            if (d != 1 && d != 3 && d != 7 && d != 9) outros += count[d];
        }
        printf("%8s | %10d | %8.2f%% | %10s\n", "outros", outros, 100.0*outros/qtd, "0.00%");
        
        int bias_37 = count[3] + count[7];
        int bias_19 = count[1] + count[9];
        printf("\nVies de Chebyshev: 3+7=%d, 1+9=%d (dif=%d)\n",
               bias_37, bias_19, bias_37 - bias_19);
        if (bias_37 > bias_19) printf("  Viés: +37 (3 e 7 predominam)\n");
        else printf("  Viés: +19 (1 e 9 predominam)\n");
        
        mpz_clears(atual, primo, gap, NULL);
        return 0;
    }
    
    if (argc >= 2 && strcmp(argv[1], "--bench") == 0) {
        /* Modo benchmark */
        int digitos = 0;
        if (argc >= 3) {
            digitos = atoi(argv[2]);
        }
        if (digitos > 0) {
            benchmark_aleatorio(digitos);
        } else {
            benchmark_aleatorio(100);
            benchmark_aleatorio(500);
            benchmark_aleatorio(1000);
            benchmark_aleatorio(2000);
            benchmark_aleatorio(3000);
        }
        return 0;
    }
    
    if (argc >= 2 && strcmp(argv[1], "--test") == 0) {
        /* Testes pre-definidos */
        teste_numero("170141183460469231731687303715884105727", "M127 (39 dig)");
        teste_numero("2", "N=2");
        teste_numero("10000000000000000000000000000000000000000", "10^40");
        return 0;
    }
    
    /* Modo interativo */
    mpz_t N, primo, gap;
    long contagens[N_FERRAMENTAS];
    clock_t t0, t1;
    
    mpz_inits(N, primo, gap, NULL);
    
    if (argc >= 2) {
        if (mpz_set_str(N, argv[1], 10) != 0) {
            fprintf(stderr, "Erro: N invalido\n");
            return 1;
        }
    } else {
        char buf[65536];
        printf("Digite N: ");
        if (!fgets(buf, sizeof(buf), stdin)) return 1;
        buf[strcspn(buf, "\n")] = 0;
        if (strlen(buf) == 0) {
            mpz_set_str(N, "170141183460469231731687303715884105727", 10);
            printf("Usando M127\n");
        } else if (mpz_set_str(N, buf, 10) != 0) {
            fprintf(stderr, "Erro: N invalido\n");
            return 1;
        }
    }
    
    {
        char *s = mpz_get_str(NULL, 10, N);
        int len = strlen(s);
        printf("\nN = ");
        if (len > 80) printf("%.40s...%.40s", s, s + len - 40);
        else printf("%s", s);
        printf("  (%d dig)\n", len);
        free(s);
    }
    
    t0 = clock();
    int r = mpz_probab_prime_p(N, 25);
    t1 = clock();
    printf("Primo? %s (%.3fs)\n",
           r == 2 ? "SIM (deterministico)" : r == 1 ? "SIM (provavel)" : "NAO",
           (double)(t1-t0)/CLOCKS_PER_SEC);
    
    t0 = clock();
    proximo_primo(primo, N, 25);
    t1 = clock();
    mpz_sub(gap, primo, N);
    
    {
        char *s = mpz_get_str(NULL, 10, primo);
        int len = strlen(s);
        printf("Prox primo: ");
        if (len > 80) printf("%.40s...%.40s", s, s + len - 40);
        else printf("%s", s);
        printf("  (%.3fs)\n", (double)(t1-t0)/CLOCKS_PER_SEC);
        free(s);
    }
    
    {
        char *gs = mpz_get_str(NULL, 10, gap);
        printf("Gap: %s (par=%s)\n", gs, mpz_even_p(gap) ? "sim" : "NAO");
        if (mpz_even_p(gap)) {
            decompor_gap(gap, contagens);
            printf("Gulosa: "); imprimir_decomposicao(contagens);
            decompor_gap_variado(gap, contagens);
            printf("Variada: "); imprimir_decomposicao(contagens);
        }
        free(gs);
    }
    
    mpz_clears(N, primo, gap, NULL);
    return 0;
}
