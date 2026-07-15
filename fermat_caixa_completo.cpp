// =========================================================================
//  FATORADOR GEOMÉTRICO COM CAIXA DE FERRAMENTAS
//  Teste empírico de cada ferramenta proposta
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>

typedef uint64_t u64;

// ─── isqrt inteira (Newton) ──────────────────────────────────────────────
static u64 isqrt(u64 n) {
    if (n == 0) return 0;
    u64 x = n;
    u64 y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

// ─── FERRAMENTA 1 + 2: âncora sqrt(N) + filtro de paridade ──────────────
static u64 ancora_paridade(u64 N) {
    u64 x = isqrt(N);
    if (x * x < N) x++;
    // Filtro de paridade: N mod 4 define paridade de x
    if ((N & 3) == 1) { if ((x & 1) == 0) x++; } // x IMPAR
    else               { if ((x & 1) != 0) x++; } // x PAR
    return x;
}

// ─── FERRAMENTA 4: crivo de resíduo quadrático ───────────────────────────
// Retorna 1 se n PODE ser quadrado perfeito, 0 se CERTAMENTE não é
static inline int crivo_residuo(u64 n) {
    // Mod 16: quadrados perfeitos só terminam em {0,1,4,9}
    unsigned m16 = n & 15;
    if (m16 > 9) return 0;
    if (m16 != 0 && m16 != 1 && m16 != 4 && m16 != 9) return 0;
    // Mod 9: quadrados perfeitos só dão resto {0,1,4,7}
    unsigned m9 = n % 9;
    if (m9 != 0 && m9 != 1 && m9 != 4 && m9 != 7) return 0;
    // Mod 7: quadrados mod 7 = {0,1,2,4}
    unsigned m7 = n % 7;
    if (m7 != 0 && m7 != 1 && m7 != 2 && m7 != 4) return 0;
    return 1; // passou em todos os crivos, PODE ser quadrado
}

// ─── MÉTODO A: Fermat básico (x += 2) ───────────────────────────────────
static u64 fermat_basico(u64 N, u64* p, u64* q, u64* iter) {
    u64 x = ancora_paridade(N);
    *iter = 0;
    while (1) {
        (*iter)++;
        u64 y2 = x * x - N;
        u64 y = isqrt(y2);
        if (y * y == y2) { *p = x - y; *q = x + y; return 1; }
        x += 2;
        if (x > (N + 1) / 2) return 0;
    }
}

// ─── MÉTODO B: Fermat + crivo de resíduo ────────────────────────────────
static u64 fermat_crivo(u64 N, u64* p, u64* q, u64* iter, u64* descartes) {
    u64 x = ancora_paridade(N);
    *iter = 0;
    *descartes = 0;
    while (1) {
        u64 y2 = x * x - N;
        // Ferramenta 4: descarta rápido sem isqrt
        if (crivo_residuo(y2)) {
            (*iter)++;
            u64 y = isqrt(y2);
            if (y * y == y2) { *p = x - y; *q = x + y; return 1; }
        } else {
            (*descartes)++;
        }
        x += 2;
        if (x > (N + 1) / 2) return 0;
    }
}

// ─── MÉTODO C: Fermat + crivo + TENTATIVA 2^s (Ferramenta 3) ────────────
//  Tenta offsets que são potências de 2 a partir de x₀.
//  AVISO: isso NÃO é correto para Fermat clássico, mas testamos mesmo assim.
static u64 fermat_caixa(u64 N, u64* p, u64* q, u64* iter) {
    u64 x0 = ancora_paridade(N);
    *iter = 0;

    // Tenta x0 + 2^s para s = 0,1,2,...,40
    for (int s = 0; s < 42; s++) {
        u64 dx = (s == 0) ? 0 : (1ULL << s);
        u64 x = x0 + dx;
        if (x > (N + 1) / 2) break;

        u64 y2 = x * x - N;
        (*iter)++;
        if (crivo_residuo(y2)) {
            u64 y = isqrt(y2);
            if (y * y == y2) { *p = x - y; *q = x + y; return 1; }
        }
    }
    return 0;
}

// ─── MÉTODO D: Caixa completa (geração de RSA consecutivos) ─────────────
//  Usa a caixa para navegar entre MÓDULOS consecutivos: N' = N + p*g
static int is_prime_mr(u64 n);
static int prescreen(u64 n);
static u64 next_prime_caixa(u64 p) {
    if (p == 2) return 3;
    for (u64 g = 2; g <= 256; g += 2) {
        u64 cand = p + g;
        if (!prescreen(cand)) continue;
        if (is_prime_mr(cand)) return cand;
    }
    return 0;
}
static void gerar_consecutivos(u64 p, u64 q, int n) {
    printf("\n  Gerando %d modulos consecutivos com p=%llu:\n", n, p);
    printf("  %-4s | %-14s | %-14s | %-8s | %s\n",
           "i", "q", "N = p*q", "gap_N", "gap_q (K(s))");
    for (int i = 0; i < n; i++) {
        u64 N = p * q;
        u64 q2 = next_prime_caixa(q);
        if (!q2) break;
        u64 gap_q = q2 - q;
        u64 gap_N = p * gap_q;
        u64 N2 = N + gap_N;
        printf("  %-4d | %-14llu | %-14llu | %-8llu | gap_q=%llu",
               i, q, N, gap_N, gap_q);
        // Decomposição gulosa do gap_q em K(s)=2^s-1
        u64 temp = gap_q;
        while (temp > 0) {
            int s = 1;
            u64 k = 1;
            while ((k << 1) + 1 <= temp) { k = (k << 1) + 1; s++; }
            printf(" K(%d)", s);
            temp -= k;
        }
        printf("\n");
        q = q2;
    }
}

// ─── Miller-Rabin + prescreen ───────────────────────────────────────────
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
    u64 bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
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

// ═════════════════════════════════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════════════════════════════════
int main() {
    u64 p_real = 3999971, q_real = 4999999;
    u64 N = p_real * q_real;  // ~2e13 (RSA-64 fraco)
    u64 p, q, iter, desc;

    printf("================================================================\n");
    printf("  TESTE DAS 4 FERRAMENTAS DA CAIXA\n");
    printf("  N = %llu = %llu x %llu (gap entre fatores: %llu)\n\n", N, p_real, q_real, q_real - p_real);

    // ── Método A: Fermat básico ──
    double t0 = omp_get_wtime();
    int ok = fermat_basico(N, &p, &q, &iter);
    double tA = omp_get_wtime() - t0;
    printf("[A] Fermat basico (x+=2):\n");
    printf("    iter=%llu  tempo=%.4fs  %s\n", iter, tA, ok ? "OK" : "FALHA");

    // ── Método B: Fermat + crivo de residuo ──
    t0 = omp_get_wtime();
    ok = fermat_crivo(N, &p, &q, &iter, &desc);
    double tB = omp_get_wtime() - t0;
    double pct_desc = 100.0 * desc / (iter + desc);
    printf("[B] Fermat + crivo residuo (Ferramenta 4):\n");
    printf("    iter=%llu  descartes=%llu (%.1f%%)  tempo=%.4fs\n",
           iter, desc, pct_desc, tB);

    // ── Método C: Fermat + saltos 2^s ──
    t0 = omp_get_wtime();
    ok = fermat_caixa(N, &p, &q, &iter);
    double tC = omp_get_wtime() - t0;
    printf("[C] Fermat + saltos 2^s (Ferramenta 3):\n");
    printf("    iter=%llu  tempo=%.4fs  %s\n", iter, tC,
           ok ? "OK (SORTE!)" : "FALHA (esperado)");

    // ── Método D: Caixa para gerar modulos consecutivos ──
    printf("\n[D] Caixa de ferramentas: navegacao entre modulos:\n");
    gerar_consecutivos(97, 101, 5);

    // ── Conclusão ──
    printf("\n─── ANALISE ───\n");
    printf(" Ferramenta 1+2 (ancora + paridade): FUNCIONAM (cortam 50%%)\n");
    printf(" Ferramenta 4 (crivo residuo):       FUNCIONA (elimina %.0f%% dos isqrt)\n", pct_desc);
    printf(" Ferramenta 3 (salto 2^s):           NAO FUNCIONA para Fermat\n");
    printf("   (p+q)/2 = %llu, x0 = %llu\n", (p_real+q_real)/2, ancora_paridade(N));
    printf("   Diferenca = %llu = offset nao-potencia-de-2\n",
           (p_real+q_real)/2 - ancora_paridade(N));
    printf("\n A caixa DE FATO funciona para navegar entre modulos consecutivos\n");
    printf(" (Metodo D): N' = N + p * gap_q, onde gap_q decomposto em K(s).\n");
    printf("================================================================\n");

    return 0;
}
