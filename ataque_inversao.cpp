// =========================================================================
//  ATAQUE DE INVERSÃO POR RESSONÂNCIA DE GRADE
//  Dados N_i consecutivos (mesmo p, q consecutivos), extrai p.
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef uint64_t u64;

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

// ─── Prescreen rápido ──────────────────────────────────────────────────
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
static int is_prime_rapido(u64 n) {
    if (!prescreen(n)) return 0;
    return is_prime(n);
}

// ─── Próximo primo (caixa) ────────────────────────────────────────────
static u64 next_prime(u64 p) {
    if (p == 2) return 3;
    for (u64 g = 2; g <= 256; g += 2) {
        u64 c = p + g;
        if (!prescreen(c)) continue;
        if (is_prime(c)) return c;
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  ATAQUE VIA DOIS MÓDULOS: ∆N = p * gap_q
// ═════════════════════════════════════════════════════════════════════════
static u64 ataque_2_modulos(u64 N1, u64 N2) {
    u64 delta = N2 - N1;
    printf("  ∆N = N2 - N1 = %llu\n", delta);

    // Tenta gaps pares pequenos (g = 2,4,6,...,max_gap)
    u64 max_gap = delta < 1000 ? delta : 1000;
    if (max_gap % 2) max_gap--;

    for (u64 g = 2; g <= max_gap; g += 2) {
        if (delta % g != 0) continue;
        u64 p_cand = delta / g;

        // p_cand deve ser primo e divider N1
        if (is_prime_rapido(p_cand) && N1 % p_cand == 0) {
            printf("  gap_q = %llu, p = %llu  (∆N / gap)\n", g, p_cand);
            return p_cand;
        }
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  ATAQUE VIA TRÊS MÓDULOS: GCD(∆N1, ∆N2) = p * GCD(g1, g2)
// ═════════════════════════════════════════════════════════════════════════
static u64 ataque_3_modulos(u64 N1, u64 N2, u64 N3) {
    u64 d1 = N2 - N1;
    u64 d2 = N3 - N2;
    u64 g = mdc(d1, d2);

    printf("  ∆N1 = %llu, ∆N2 = %llu\n", d1, d2);
    printf("  GCD(∆N1, ∆N2) = %llu\n", g);

    // GCD = p * GCD(gap1, gap2). GCD(gap1,gap2) é par e pequeno (tipicamente 2).
    // p = GCD / gcd_gaps
    for (u64 divisor = 2; divisor <= g && divisor <= 64; divisor += 2) {
        if (g % divisor != 0) continue;
        u64 p_cand = g / divisor;
        if (is_prime_rapido(p_cand) && N1 % p_cand == 0) {
            printf("  divisor_gaps = %llu (GCD(gap1,gap2)), p = %llu\n", divisor, p_cand);
            return p_cand;
        }
    }
    // Se não achou com divisor pequeno, p pode ser o próprio GCD
    if (is_prime_rapido(g) && N1 % g == 0) {
        printf("  p = GCD = %llu\n", g);
        return g;
    }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════
//  CENÁRIO DE TESTE
// ═════════════════════════════════════════════════════════════════════════
int main() {
    printf("================================================================\n");
    printf("  ATAQUE DE INVERSÃO POR RESSONÂNCIA DE GRADE\n");
    printf("================================================================\n\n");

    // ── Gerar cenário: p fixo, 4 q consecutivos ──
    u64 p_secreto = 4294967291ULL; // 32-bit
    u64 q1 = 1000000007ULL;
    u64 q2 = next_prime(q1);
    u64 q3 = next_prime(q2);
    u64 q4 = next_prime(q3);

    u64 N1 = p_secreto * q1;
    u64 N2 = p_secreto * q2;
    u64 N3 = p_secreto * q3;
    u64 N4 = p_secreto * q4;

    printf("Cenario gerado:\n");
    printf("  p (secreto) = %llu\n", p_secreto);
    printf("  q1 = %llu, q2 = %llu, q3 = %llu, q4 = %llu\n", q1, q2, q3, q4);
    printf("  N1 = %llu\n", N1);
    printf("  N2 = %llu\n", N2);
    printf("  N3 = %llu\n", N3);
    printf("  N4 = %llu\n\n", N4);

    // ── Ataque 1: 2 módulos ──
    printf("[Ataque 1] Dois modulos (N1, N2):\n");
    u64 p_achado = ataque_2_modulos(N1, N2);
    printf("  Resultado: %s\n\n", p_achado ? "SUCESSO!" : "FALHA");

    // ── Ataque 2: 3 módulos ──
    printf("[Ataque 2] Tres modulos (N1, N2, N3):\n");
    p_achado = ataque_3_modulos(N1, N2, N3);
    printf("  Resultado: %s\n\n", p_achado && p_achado == p_secreto ? "SUCESSO!" : "FALHA");

    // ── Ataque 3: mostrar a progressão ──
    printf("[Ataque 3] Progressao dos gaps:\n");
    printf("  q: %llu -> %llu (gap=%llu) -> %llu (gap=%llu) -> %llu (gap=%llu)\n",
           q1, q2, q2-q1, q3, q3-q2, q4, q4-q3);
    printf("  N: %llu -> %llu (∆=%llu) -> %llu (∆=%llu) -> %llu (∆=%llu)\n\n",
           N1, N2, N2-N1, N3, N3-N2, N4, N4-N3);

    printf("  Verificacao:\n");
    printf("  p * gap_q1 = %llu * %llu = %llu\n", p_secreto, q2-q1, p_secreto*(q2-q1));
    printf("  ∆N1        = %llu\n\n", N2-N1);

    printf("  GCD(∆N1, ∆N2) = GCD(%llu, %llu) = %llu\n", N2-N1, N3-N2, mdc(N2-N1, N3-N2));
    printf("  p * GCD(g1, g2) = %llu * GCD(%llu, %llu) = %llu * %llu = %llu\n",
           p_secreto, q2-q1, q3-q2, p_secreto, mdc(q2-q1, q3-q2), p_secreto*mdc(q2-q1, q3-q2));

    // ── Ataque 4: GCD clássico entre N1 e N2 (padrão ouro) ──
    printf("[Ataque 4] GCD(N1, N2) classico:\n");
    u64 g = mdc(N1, N2);
    printf("  GCD(%llu, %llu) = %llu\n", N1, N2, g);
    printf("  (p mesmo?! %s)\n", g == p_secreto ? "SIM, p compartilhado!" : "NAO");

    printf("\n================================================================\n");
    printf("  COMPARACAO DOS ATAQUES\n");
    printf("================================================================\n");
    printf("\n");
    printf("  Metodo              | Busca          | Iteracoes\n");
    printf("  --------------------|----------------|----------\n");
    printf("  Trial division (+2) | sqrt(N) ~ 2e9  | 1e9\n");
    printf("  Fermat paridade     | |p-q|/2 ~ 5e8 | 5e8\n");
    printf("  Ataque por ∆N (2 N) | max_gap ~ 500  | 250\n");
    printf("  Ataque por GCD (3 N)| 1 iteracao     | 1\n");
    printf("  GCD classico        | 1 chamada MDC  | 1\n");
    printf("\n");
    printf("  Conclusao: com 2 modulos, a inversao por gap\n");
    printf("  reduz a busca de ~1e9 para ~250 candidatos.\n");
    printf("  Com 3 modulos, GCD da diretamente p * gcd(g1,g2).\n");
    printf("================================================================\n");

    return 0;
}
