// =========================================================================
//  ATAQUE RSA - COMPARAÇÃO DE MÉTODOS
//  Inclui a Caixa de Valências (versão correta: pré-triagem + MR)
//  e benchmark contra +2 stepping e crivo + OMP.
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>

typedef uint64_t u64;

// ─── MILLER-RABIN DETERMINÍSTICO (64-bit) ────────────────────────────────
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

// ─── CRIVO ────────────────────────────────────────────────────────────────
static int sieve(u64 limit, u64** out) {
    char* is_p = (char*)calloc(limit + 1, 1);
    if (!is_p) return 0;
    for (u64 i = 2; i <= limit; i++) is_p[i] = 1;
    for (u64 i = 2; i * i <= limit; i++)
        if (is_p[i])
            for (u64 j = i * i; j <= limit; j += i)
                is_p[j] = 0;
    int cnt = 0;
    for (u64 i = 2; i <= limit; i++) if (is_p[i]) cnt++;
    u64* p = (u64*)malloc(cnt * sizeof(u64));
    if (!p) { free(is_p); return 0; }
    int idx = 0;
    for (u64 i = 2; i <= limit; i++) if (is_p[i]) p[idx++] = i;
    free(is_p);
    *out = p;
    return idx;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODO 1: +2 STEPPING (BASELINE)
// ═════════════════════════════════════════════════════════════════════════
static u64 method_plus2(u64 N, u64* tested) {
    *tested = 0;
    u64 lim = (u64)sqrt((double)N);
    for (u64 c = 3; c <= lim; c += 2) {
        (*tested)++;
        if (N % c == 0) return c;
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODO 2: CAIXA DE VALÊNCIAS (ALGORITMO CORRETO)
//
//  Para encontrar o próximo primo a partir de p:
//    1. Tenta gaps pares consecutivos: 2, 4, 6, 8, ...
//    2. Pré-triagem: testa divisibilidade por primos ≤ 29
//    3. Se passar, Miller-Rabin determinístico
//  A decomposição do gap em K(s)=2^s-1 é pós-processamento (não usada aqui)
// ═════════════════════════════════════════════════════════════════════════

// Pequenos primos para pré-triagem
static const int small_primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };
static const int n_small = 10;

static inline int prescreen(u64 n) {
    for (int i = 1; i < n_small; i++) { // skip 2 (n is odd)
        if (small_primes[i] >= n) break;
        if (n % small_primes[i] == 0) return 0;
    }
    return 1;
}

// Encontra o próximo primo usando gaps pares + pré-triagem + MR
static u64 next_prime_caixa(u64 p, u64 max_gap) {
    if (p == 2) return 3;
    for (u64 gap = 2; gap <= max_gap; gap += 2) {
        u64 cand = p + gap;
        if (!prescreen(cand)) continue;
        if (is_prime_mr(cand)) return cand;
    }
    return 0; // excedeu max_gap
}

// Navegação sequencial via caixa
static u64 method_caixa(u64 N, u64* tested) {
    *tested = 0;
    u64 lim = (u64)sqrt((double)N);
    u64 p = 3;
    u64 max_gap = 256; // max gap esperado para números até 2^32
    while (p <= lim) {
        (*tested)++;
        if (N % p == 0) return p;
        p = next_prime_caixa(p, max_gap);
        if (p == 0) break; // gap excedeu limite
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODO 3: CAIXA + OMP (paralelo)
// ═════════════════════════════════════════════════════════════════════════
static u64 method_caixa_omp(u64 N, u64* tested) {
    *tested = 0;
    u64 lim = (u64)sqrt((double)N);
    volatile u64 found = 0;
    u64 max_gap = 256;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int nth = omp_get_num_threads();
        // Cada thread começa de um ímpar diferente e avança até o primeiro primo
        u64 start = 3 + 2 * (u64)tid;
        // Avança até o primeiro primo no seu range
        while (start <= lim && !found) {
            if (prescreen(start) && is_prime_mr(start)) break;
            start += 2 * nth;
        }
        u64 p = start;
        while (p <= lim && !found) {
            if (N % p == 0) {
                #pragma omp atomic write
                found = p;
                break;
            }
            #pragma omp atomic update
            (*tested)++;
            p = next_prime_caixa(p, max_gap);
            if (p == 0) break;
        }
    }
    return found;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODO 4: CRIVO + DIVISÃO PARALELA (MAIS RÁPIDO)
// ═════════════════════════════════════════════════════════════════════════
static u64 method_sieve_omp(u64 N, u64* tested) {
    *tested = 0;
    u64 lim = (u64)sqrt((double)N);
    u64* primes;
    int np = sieve(lim, &primes);
    if (!np) return 0;
    volatile u64 found = 0;

    #pragma omp parallel for schedule(dynamic, 10000)
    for (int i = 0; i < np; i++) {
        if (found) continue;
        if (N % primes[i] == 0) {
            #pragma omp atomic write
            found = primes[i];
        }
        #pragma omp atomic update
        (*tested)++;
    }
    free(primes);
    return found;
}

// ═════════════════════════════════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════════════════════════════════
int main() {
    u64 p_real = 3999971, q_real = 4999999;
    u64 N = p_real * q_real;
    u64 tested;
    double dt[4];
    u64 factors[4];

    printf("==============================================================\n");
    printf("  RSA FACTOR ATTACK  |  N = %llu\n", N);
    printf("  Fatores reais: %llu x %llu\n", p_real, q_real);
    printf("==============================================================\n\n");

    // ── 1) +2 stepping ──
    double t0 = omp_get_wtime();
    factors[0] = method_plus2(N, &tested);
    dt[0] = omp_get_wtime() - t0;
    printf("[1] +2 stepping (seq):\n");
    printf("    fator=%llu  q=%llu  testes=%llu  tempo=%.4fs\n",
           factors[0], N/factors[0], tested, dt[0]);

    // ── 2) Caixa sequencial ──
    t0 = omp_get_wtime();
    factors[1] = method_caixa(N, &tested);
    dt[1] = omp_get_wtime() - t0;
    printf("[2] Caixa (seq, MR):\n");
    printf("    fator=%llu  q=%llu  testes=%llu  tempo=%.4fs\n",
           factors[1], N/factors[1], tested, dt[1]);

    // ── 3) Caixa + OMP ──
    t0 = omp_get_wtime();
    factors[2] = method_caixa_omp(N, &tested);
    dt[2] = omp_get_wtime() - t0;
    printf("[3] Caixa + OMP:\n");
    printf("    fator=%llu  q=%llu  testes=%llu  tempo=%.4fs\n",
           factors[2], N/factors[2], tested, dt[2]);

    // ── 4) Crivo + OMP ──
    t0 = omp_get_wtime();
    factors[3] = method_sieve_omp(N, &tested);
    dt[3] = omp_get_wtime() - t0;
    printf("[4] Crivo + OMP:\n");
    printf("    fator=%llu  q=%llu  testes=%llu  tempo=%.4fs\n",
           factors[3], N/factors[3], tested, dt[3]);

    // ── Comparação ──
    printf("\n─── COMPARAÇÃO ───\n");
    printf("  +2 stepping  : %.4fs  (1.00x)\n", dt[0]);
    printf("  Caixa seq    : %.4fs  (%.2fx)\n", dt[1], dt[0]/dt[1]);
    printf("  Caixa OMP    : %.4fs  (%.2fx)\n", dt[2], dt[1]/dt[2] > 0 ? dt[0]/dt[2] : 0);
    printf("  Crivo + OMP  : %.4fs  (%.2fx)\n\n", dt[3], dt[0]/dt[3]);

    printf("─── ANÁLISE ───\n");
    printf("  +2 stepping: %lld divisoes (N %% c), sem primalidade\n", tested);
    printf("  Caixa: cada primo requer pre-triagem + MR (centenas de mul/div)\n");
    printf("  Caixa NAO acelera fatoracao vs +2 simples.\n");
    printf("  Seu valor: gerar primos para CONSTRUIR chaves RSA.\n");

    return 0;
}
