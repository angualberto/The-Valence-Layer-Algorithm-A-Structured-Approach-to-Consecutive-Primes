// =========================================================================
//  BENCHMARK: Ataque RSA em múltiplos tamanhos (8, 16, 32, 48, 64 bits)
//  Compara: Trial division, Fermat, Inversão por gap, GCD
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>

typedef uint64_t u64;
typedef unsigned __int128 u128;

// ─── isqrt ───────────────────────────────────────────────────────────────
static u64 isqrt(u64 n) {
    if (!n) return 0;
    u64 x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}
// ─── MDC ────────────────────────────────────────────────────────────────
static u64 mdc(u64 a, u64 b) {
    while (b) { u64 t = a % b; a = b; b = t; }
    return a;
}

// ─── Miller-Rabin ───────────────────────────────────────────────────────
static inline u64 mul_mod(u64 a, u64 b, u64 m) {
    return ((u128)a * b) % m;
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
static int is_prime(u64 n) {
    if (n < 2) return 0;
    if (n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    if (n % 5 == 0) return n == 5;
    if (n % 7 == 0) return n == 7;
    u64 d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; s++; }
    u64 bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
    for (int i = 0; i < 7; i++) {
        u64 a = bases[i] % n;
        if (!a) continue;
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
static int prescreen(u64 n) {
    if (n % 3 == 0) return n == 3;
    if (n % 5 == 0) return n == 5;
    if (n % 7 == 0) return n == 7;
    if (n % 11 == 0) return n == 11;
    if (n % 13 == 0) return n == 13;
    if (n % 17 == 0) return n == 17;
    if (n % 19 == 0) return n == 19;
    if (n % 23 == 0) return n == 23;
    if (n % 29 == 0) return n == 29;
    return 1;
}
static int is_prime_fast(u64 n) {
    return prescreen(n) && is_prime(n);
}

// ─── Gerador de primos ─────────────────────────────────────────────────
static u64 next_prime(u64 p) {
    if (p == 2) return 3;
    for (u64 g = 2; g <= 256; g += 2) {
        u64 c = p + g;
        if (!prescreen(c)) continue;
        if (is_prime(c)) return c;
    }
    return 0;
}

// ─── Gera primo de 'bits' bits ─────────────────────────────────────────
static u64 gen_prime(int bits) {
    u64 min = bits > 1 ? (1ULL << (bits - 1)) : 2;
    u64 max = (1ULL << bits) - 1;
    if (bits >= 64) { min = 0x8000000000000001ULL; max = 0xFFFFFFFFFFFFFFFFULL; }
    // Começa de um ponto conhecido e acha o próximo primo
    u64 start = min;
    if (!prescreen(start) || !is_prime(start)) {
        if (start % 2 == 0) start++;
        while (!is_prime_fast(start)) start += 2;
    }
    return start;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODOS DE ATAQUE
// ═════════════════════════════════════════════════════════════════════════

// 1) Trial division (+2)
static u64 atk_trial(u64 N, u64* iter) {
    *iter = 0;
    u64 lim = isqrt(N);
    for (u64 c = 3; c <= lim; c += 2) {
        (*iter)++;
        if (N % c == 0) return c;
    }
    return 0;
}

// 2) Fermat + paridade
static u64 atk_fermat(u64 N, u64* iter) {
    *iter = 0;
    u64 x = isqrt(N);
    if (x * x < N) x++;
    // paridade
    if ((N & 3) == 1) { if ((x & 1) == 0) x++; }
    else              { if ((x & 1) != 0) x++; }
    while (1) {
        (*iter)++;
        u64 y2 = x * x - N;
        u64 y = isqrt(y2);
        if (y * y == y2) return x - y;
        x += 2;
        if (x > (N + 1) / 2) return 0;
    }
}

// 3) Inversão por gap (precisa de N1, N2 com mesmo p)
static u64 atk_gap(u64 N1, u64 N2, u64* iter) {
    *iter = 0;
    u64 delta = N2 - N1;
    u64 max_gap = delta < 500 ? delta : 500;
    if (max_gap % 2) max_gap--;
    for (u64 g = 2; g <= max_gap; g += 2) {
        (*iter)++;
        if (delta % g != 0) continue;
        u64 p_cand = delta / g;
        if (is_prime_fast(p_cand) && N1 % p_cand == 0) return p_cand;
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════════════════════════════════
int main() {
    struct { int bits; char* label; } tests[] = {
        {8,  "8-bit"}, {16, "16-bit"}, {24, "24-bit"},
        {32, "32-bit"}, {40, "40-bit"}, {48, "48-bit"}, {56, "56-bit"}
    };
    int n_tests = sizeof(tests) / sizeof(tests[0]);

    printf("================================================================\n");
    printf("  BENCHMARK: METODOS DE ATAQUE POR TAMANHO DE PRIMO\n");
    printf("================================================================\n\n");
    printf("  %-10s | %-12s | %-12s | %-12s | %-12s | %-12s\n",
           "bits", "Trial(iter)", "Trial(temp)", "Fermat(iter)", "Gap(iter)", "Gap(temp)");
    printf("  %-10s-|-%12s-|-%12s-|-%12s-|-%12s-|-%12s\n",
           "----------", "------------", "------------", "------------", "------------", "------------");

    for (int t = 0; t < n_tests; t++) {
        int bits = tests[t].bits;
        // Primos > 32-bit: N > 2^64, nao cabe em u64
        if (bits > 32) {
            printf("  %-10s | %-12s | %-12s | %-12s | %-12s | %-12s\n",
                   tests[t].label, "N/A (u64)", "overflow", "N/A", "N/A", "N/A");
            continue;
        }

        // Gera p,q com gaps pequenos (Fermat viável)
        u64 p = gen_prime(bits);
        u64 q = p + 2;
        while (!is_prime_fast(q)) q += 2;
        // Para Fermat ser viável, q precisa estar próximo de p
        u64 q2 = next_prime(q);

        u64 N1 = p * q;
        u64 N2 = p * q2;

        if (N1 < p || N2 < p || N1 == 0 || N2 == 0) {
            printf("  %-10s | %-12s | %-12s | %-12s | %-12s | %-12s\n",
                   tests[t].label, "OVERFLOW", "OVERFLOW", "OVERFLOW", "OVERFLOW", "OVERFLOW");
            continue;
        }

        u64 it1, it2, it3;
        double t0;

        t0 = omp_get_wtime();
        u64 p1 = atk_trial(N1, &it1);
        double dt1 = omp_get_wtime() - t0;

        t0 = omp_get_wtime();
        u64 p2 = atk_fermat(N1, &it2);
        double dt2 = omp_get_wtime() - t0;

        t0 = omp_get_wtime();
        u64 p3 = atk_gap(N1, N2, &it3);
        double dt3 = omp_get_wtime() - t0;

        printf("  %-10s | %-12llu | %-9.6fs | %-12llu | %-12llu | %-9.6fs",
               tests[t].label,
               (unsigned long long)it1, dt1,
               (unsigned long long)it2,
               (unsigned long long)it3, dt3);

        // Validação
        char v1 = (p1 == p || p1 == q) ? 'V' : 'X';
        char v2 = (p2 == p || p2 == q) ? 'V' : 'X';
        char v3 = (p3 == p) ? 'V' : 'X';
        printf(" [%c%c%c]\n", v1, v2, v3);
    }

    printf("\n─── LEGENDA ───\n");
    printf("  Trial: trial division (+2), Fermat: Fermat + paridade\n");
    printf("  Gap: inversao por ∆N = N2 - N1 = p * gap_q\n");
    printf("  [V]: encontrou fator, [X]: falhou\n");

    printf("\n─── COMPLEXIDADE TEORICA ───\n");
    printf("  Trial: O(sqrt(N)) ~ O(2^(bits))    → exponencial no bits\n");
    printf("  Fermat: O(|p-q|/2)                  → depende do gap entre fatores\n");
    printf("  Gap: O(max_gap) ~ O(1)              → ~250 iteracoes, ANY size\n");
    printf("  GCD(N1,N2): O(log N)                → trivial, mas requer p identico\n");
    printf("\n================================================================\n");

    return 0;
}
