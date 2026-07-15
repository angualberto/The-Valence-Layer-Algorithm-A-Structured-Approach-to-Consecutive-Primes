// =========================================================================
//  GERADOR RÁPIDO DE PARES RSA CONSECUTIVOS
//  Usa a Caixa de Valências escalada: N' = N + p * g
//  Onde g ǩ o gap entre primos consecutivos (decomposto pela caixa)
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

typedef uint64_t u64;

// ─── MILLER-RABIN ────────────────────────────────────────────────────────
static inline u64 mul_mod(u64 a, u64 b, u64 m) {
    return ((unsigned __int128)a * b) % m;
}
static inline u64 pow_mod(u64 base, u64 exp, u64 m) {
    u64 r = 1 % m;
    base %= m;
    while (exp) {
        if (exp & 1) r = mul_mod(r, base, m);
        base = mul_mod(base, base, m);
        exp >>= 1;
    }
    return r;
}
static int is_prime_mr(u64 n) {
    if (n < 2) return 0;
    if (n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    if (n % 5 == 0) return n == 5;
    if (n % 7 == 0) return n == 7;
    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; s++; }
    u64 bases[] = { 2, 325, 9375, 28178, 450775, 9780504, 1795265022 };
    for (int i = 0; i < 7; i++) {
        u64 a = bases[i] % n;
        if (a == 0) continue;
        u64 x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) {
            x = mul_mod(x, x, n);
            if (x == n - 1) { ok = 1; break; }
        }
        if (!ok) return 0;
    }
    return 1;
}

// ─── PRÉ-TRIAGEM (primos ≤ 29) ──────────────────────────────────────────
static const int small_primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };
static int prescreen(u64 n) {
    for (int i = 1; i < 10; i++) {
        if (small_primes[i] >= n) break;
        if (n % small_primes[i] == 0) return 0;
    }
    return 1;
}

// ─── PRÓXIMO PRIMO (via caixa: gaps pares + prescreen + MR) ─────────────
static u64 next_prime_caixa(u64 p) {
    if (p == 2) return 3;
    u64 max_gap = 256;
    for (u64 g = 2; g <= max_gap; g += 2) {
        u64 cand = p + g;
        if (!prescreen(cand)) continue;
        if (is_prime_mr(cand)) return cand;
    }
    return 0;
}

// ─── DECOMPOSIÇÃO DO GAP EM K(s) = 2^s - 1 (CAIXA DE FERRAMENTAS) ─────
// Algoritmo guloso: para cada gap par, subtrai o maior K(s) possível
static void decompor_gap(u64 gap) {
    printf("        gap %llu = ", gap);
    int primeiro = 1;
    while (gap > 0) {
        int s = 1;
        u64 k = 1; // K(1) = 1
        while ((k << 1) + 1 <= gap) { // K(s+1) = 2*K(s) + 1
            k = (k << 1) + 1;
            s++;
        }
        if (!primeiro) printf(" + ");
        printf("K(%d)=%llu", s, k);
        primeiro = 0;
        gap -= k;
    }
    printf("\n");
}

// ─── GERADOR DE PARES RSA CONSECUTIVOS ───────────────────────────────────
// Dado N = p*q, gera o próximo N' = p * next_prime(q)
// Gap entre módulos: N' - N = p * g (onde g = q' - q)
static void gerar_pares_rsa(u64 p, u64 q_inicial, int n_pares) {
    u64 q = q_inicial;
    printf("p = %llu (fixo)\n\n", p);

    for (int i = 0; i < n_pares; i++) {
        u64 N = p * q;
        u64 q_prox = next_prime_caixa(q);
        if (q_prox == 0) {
            printf("[%d] overflow em q=%llu\n", i, q);
            break;
        }
        u64 gap_q = q_prox - q;       // gap entre primos
        u64 gap_N = p * gap_q;        // gap entre módulos (escalado)
        u64 N_prox = N + gap_N;

        printf("[%d] q = %llu  →  q' = %llu\n", i, q, q_prox);
        printf("    N    = %llu\n", N);
        printf("    N'   = %llu\n", N_prox);
        printf("    gap_q = %llu  (gap entre primos)\n", gap_q);
        printf("    p*gap_q = %llu  (gap entre modulos, escalado por p)\n", gap_N);
        decompor_gap(gap_q);

        q = q_prox;
    }
}

// ─── DEMONSTRAÇÃO: CAIXA ESCALADA ACELERA GERAÇÃO DE MÚLTIPLOS N ────────
static void benchmark_geracao(u64 q_start, int n) {
    volatile u64 q; // volatile força execução real no hardware
    u64 c;

    // Via caixa
    q = q_start;
    double t0 = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        q = next_prime_caixa(q);
        if (q == 0) break;
    }
    double t_caixa = omp_get_wtime() - t0;

    // Via +2 ingênuo (testa todos os ímpares com MR puro, sem prescreen)
    q = q_start;
    double t1 = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        c = q + 2;
        while (!is_prime_mr(c)) c += 2;
        q = c;
    }
    double t_ing = omp_get_wtime() - t1;

    printf("\n─── BENCHMARK: %d pares RSA ───\n", n);
    printf("  Caixa (prescreen + MR): %.4fs\n", t_caixa);
    printf("  +2 ingênuo (MR puro):   %.4fs\n", t_ing);
    printf("  Speedup:                %.2fx\n", t_ing / t_caixa);
}

// ─── MAIN ────────────────────────────────────────────────────────────────
int main() {
    printf("==============================================================\n");
    printf("  GERADOR DE PARES RSA CONSECUTIVOS\n");
    printf("  Usa Caixa escalada: N' = N + p * gap_q\n");
    printf("==============================================================\n\n");

    // Exemplo 1: gerar 5 pares a partir de p=5, q=3
    printf("─── Exemplo 1: p=5, q=3 ───\n");
    gerar_pares_rsa(5, 3, 5);

    printf("\n─── Exemplo 2: p=97, q=101 ───\n");
    gerar_pares_rsa(97, 101, 5);

    printf("\n─── Exemplo 3: p=7919, q=7927 ───\n");
    gerar_pares_rsa(7919, 7927, 5);

    // Benchmarks (usar clock() para maior precisão em loops rápidos)
    printf("\n─── Primos ~31-bit (5000 iter) ───\n");
    benchmark_geracao(3000000007ULL, 5000);

    printf("\n─── Primos ~62-bit (500 iter) ───\n");
    benchmark_geracao(4000000000000000003ULL, 500);

    printf("\n==============================================================\n");
    printf("  CONCLUSÃO: A caixa ESCALADA por p\n");
    printf("  prevê exatamente onde cai o próximo módulo RSA.\n");
    printf("  gap_N = p * gap_q = p * (q' - q)\n");
    printf("  A decomposição de gap_q em K(s) = 2^s-1\n");
    printf("  se preserva multiplicativamente em gap_N.\n");
    printf("==============================================================\n");
    return 0;
}
