// =========================================================================
//  BENCHMARK RSA 8..64 bits com u128 (concatenação de 2 limbs)
//  N = p * q  cabe em u128 até primos de 64 bits.
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <omp.h>

typedef uint64_t u64;
typedef unsigned __int128 u128;

// ─── isqrt para u128 ────────────────────────────────────────────────────
static u128 isqrt128(u128 n) {
    if (!n) return 0;
    u128 x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

// ─── MDC para u64 ───────────────────────────────────────────────────────
static u64 mdc64(u64 a, u64 b) {
    while (b) { u64 t = a % b; a = b; b = t; }
    return a;
}

// ─── Miller-Rabin (u64) ─────────────────────────────────────────────────
static inline u64 mul_mod64(u64 a, u64 b, u64 m) {
    return ((u128)a * b) % m;
}
static inline u64 pow_mod64(u64 base, u64 exp, u64 m) {
    u64 r = 1 % m;
    base %= m;
    while (exp) {
        if (exp & 1) r = mul_mod64(r, base, m);
        base = mul_mod64(base, base, m);
        exp >>= 1;
    }
    return r;
}
static int is_prime64(u64 n) {
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
        u64 x = pow_mod64(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) {
            x = mul_mod64(x, x, n);
            if (x == n - 1) { ok = 1; break; }
        }
        if (!ok) return 0;
    }
    return 1;
}
static int prescreen64(u64 n) {
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
static int is_prime64_fast(u64 n) {
    return prescreen64(n) && is_prime64(n);
}
static u64 next_prime64(u64 p) {
    if (p == 2) return 3;
    for (u64 g = 2; g <= 256; g += 2) {
        u64 c = p + g;
        if (!prescreen64(c)) continue;
        if (is_prime64(c)) return c;
    }
    return 0;
}

// ─── Gera primo de 'bits' bits ─────────────────────────────────────────
static u64 gen_prime64(int bits) {
    if (bits < 2) return 2;
    u64 min = bits >= 64 ? 0x8000000000000001ULL : (1ULL << (bits - 1));
    u64 start = min;
    if (start % 2 == 0) start++;
    while (!is_prime64_fast(start)) start += 2;
    return start;
}

// ═════════════════════════════════════════════════════════════════════════
//  MÉTODOS DE ATAQUE (usando u128 para N)
// ═════════════════════════════════════════════════════════════════════════

// 1) Trial division (+2) sobre u128
static u64 atk_trial128(u128 N, u64* iter) {
    *iter = 0;
    u128 lim = isqrt128(N);
    for (u64 c = 3; c <= (u64)lim; c += 2) {
        (*iter)++;
        if (N % c == 0) return c;
    }
    return 0;
}

// 2) Fermat + paridade sobre u128
static u64 atk_fermat128(u128 N, u64* iter) {
    *iter = 0;
    u128 x = isqrt128(N);
    if (x * x < N) x++;
    if ((N & 3) == 1) { if ((x & 1) == 0) x++; }
    else              { if ((x & 1) != 0) x++; }
    while (1) {
        (*iter)++;
        u128 y2 = x * x - N;
        u128 y = isqrt128(y2);
        if (y * y == y2) return (u64)(x - y);
        x += 2;
        if (x > (N + 1) / 2) return 0;
    }
}

// 3) Inversão por gap sobre u128
static u64 atk_gap128(u128 N1, u128 N2, u64* iter) {
    *iter = 0;
    u128 delta = N2 - N1;
    u64 max_gap = delta < 500 ? (u64)delta : 500;
    if (max_gap % 2) max_gap--;
    for (u64 g = 2; g <= max_gap; g += 2) {
        (*iter)++;
        if (delta % g != 0) continue;
        u64 p_cand = (u64)(delta / g);
        if (is_prime64_fast(p_cand) && N1 % p_cand == 0) return p_cand;
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════════════════════════════════
int main() {
    int bits_list[] = {8, 16, 24, 32, 40, 48};
    int n_tests = sizeof(bits_list) / sizeof(bits_list[0]);

    printf("================================================================\n");
    printf("  BENCHMARK RSA (u128) — TAMANHOS 8..48 BITS\n");
    printf("  N = p * q armazenado em u128 (concatenação de 2 limbs)\n");
    printf("  >48-bit: trial inviável, Fermat lento, GAP continua O(1)\n");
    printf("================================================================\n\n");

    printf("  %-9s | %-12s | %-12s | %-12s | %-12s | %-12s | %s\n",
           "bits(p)", "Trial(iter)", "Trial(temp)", "Fermat(iter)", "Gap(iter)",
           "Gap(temp)", "Status");
    printf("  %-9s-|-%12s-|-%12s-|-%12s-|-%12s-|-%12s-|-%s\n",
           "---------", "------------", "------------", "------------",
           "------------", "------------", "--------");

    for (int t = 0; t < n_tests; t++) {
        int bits = bits_list[t];

        // Gera p, q próximos (gap pequeno)
        u64 p = gen_prime64(bits);
        u64 q = p + 2;
        while (!is_prime64_fast(q)) q += 2;
        u64 q2 = next_prime64(q);

        u128 N1 = (u128)p * q;
        u128 N2 = (u128)p * q2;

        u64 it1 = 0, it2 = 0, it3 = 0;
        double dt1 = 0, dt2 = 0, dt3 = 0;
        char status[8] = "---";
        int ok1 = 0, ok2 = 0, ok3 = 0;

        double t0;

        // Trial (timeout para bits grandes)
        if (bits <= 40) {
            t0 = omp_get_wtime();
            u64 p1 = atk_trial128(N1, &it1);
            dt1 = omp_get_wtime() - t0;
            ok1 = (p1 == p || p1 == q);
        } else {
            it1 = 0; dt1 = 0; status[0] = 'T'; // Trial timeout
        }

        // Fermat
        if (bits <= 56) {
            t0 = omp_get_wtime();
            u64 p2 = atk_fermat128(N1, &it2);
            dt2 = omp_get_wtime() - t0;
            ok2 = (p2 == p || p2 == q);
        } else {
            it2 = 0; dt2 = 0; status[1] = 'F';
        }

        // Gap (sempre rápido, independente do tamanho)
        t0 = omp_get_wtime();
        u64 p3 = atk_gap128(N1, N2, &it3);
        dt3 = omp_get_wtime() - t0;
        ok3 = (p3 == p);

        char v1 = ok1 ? 'V' : (bits <= 40 ? 'X' : '-');
        char v2 = ok2 ? 'V' : (bits <= 56 ? 'X' : '-');
        char v3 = ok3 ? 'V' : 'X';
        char label[16]; sprintf(label, "%d-bit", bits);

        printf("  %-9s | %-12llu | %-9.4fs | %-12llu | %-12llu | %-9.4fs | [%c%c%c]\n",
               label,
               (unsigned long long)it1, dt1,
               (unsigned long long)it2,
               (unsigned long long)it3, dt3, v1, v2, v3);
    }

    printf("\n─── LEGENDA ───\n");
    printf("  Trial: trial division (+2) sobre u128\n");
    printf("  Fermat: Fermat + paridade sobre u128\n");
    printf("  Gap: inversao por delta_N = N2 - N1 = p * gap_q\n");
    printf("  V: achou fator, X: falhou, -: nao executado (timeout)\n");

    printf("\n─── ANALISE ───\n");
    printf("  Trial: O(sqrt(N)) ~ 2^bits iteracoes → exponencial\n");
    printf("  Fermat: O(|p-q|/2) → O(1) para primos proximos\n");
    printf("  Gap: O(max_gap) ~ O(1) → 1 a 250 iteracoes, QUALQUER tamanho\n");
    printf("  u128 cobre ate RSA-128 (primos 64-bit). Para >128-bit,\n");
    printf("  usar GMP ou bigint limb (concatenacao de N limbs).\n");
    printf("================================================================\n");

    return 0;
}
