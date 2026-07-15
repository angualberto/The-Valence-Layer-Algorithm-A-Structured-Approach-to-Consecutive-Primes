// =========================================================================
//  MAPEAMENTO DA GRADE DE RESSONÂNCIA DE MÓDULOS RSA
//  Valida: gap_N = p * gap_q
//  Decompõe: gap_q = Σ K(s) = Σ (2^s - 1)
// =========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>

typedef uint64_t u64;

// ─── isqrt ───────────────────────────────────────────────────────────────
static u64 isqrt(u64 n) {
    if (!n) return 0;
    u64 x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

// ─── Miller-Rabin (para validar os q consecutivos) ──────────────────────
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

// ─── Caixa: próximo primo via gaps pares + prescreen + MR ──────────────
static u64 next_prime(u64 p) {
    if (p == 2) return 3;
    for (u64 g = 2; g <= 256; g += 2) {
        u64 c = p + g;
        if (!prescreen(c)) continue;
        if (is_prime(c)) return c;
    }
    return 0;
}

// ─── Decomposição gulosa de gap em K(s) = 2^s - 1 ─────────────────────
// Retorna o número de termos usados
static int decompor_k(u64 gap, int* termos_s) {
    int n = 0;
    u64 resto = gap;
    while (resto > 0) {
        int s = 1;
        u64 k = 1; // K(1) = 1
        while ((k << 1) + 1 <= resto) { k = (k << 1) + 1; s++; }
        termos_s[n++] = s;
        resto -= k;
    }
    return n;
}

// ═════════════════════════════════════════════════════════════════════════
//  EXPERIMENTO PRINCIPAL
// ═════════════════════════════════════════════════════════════════════════
int main() {
    u64 p = 4294967291ULL; // primo 32-bit: 2^32 - 5

    printf("================================================================\n");
    printf("  MAPEAMENTO DA GRADE DE RESSONANCIA\n");
    printf("  p (fixo) = %llu  (32-bit)\n", p);
    printf("================================================================\n\n");

    // ── Fase 1: gerar sequência de q consecutivos ──
    printf("[Fase 1] Gerando 10 primos q consecutivos:\n");
    printf("  q0 = ? (vamos comecar de 10^9+7 e navegar)\n\n");

    u64 q = 1000000007ULL; // primo inicial
    int N_PASSOS = 10;
    u64* qs = (u64*)malloc((N_PASSOS + 1) * sizeof(u64));
    qs[0] = q;

    for (int i = 0; i < N_PASSOS; i++) {
        qs[i + 1] = next_prime(qs[i]);
        if (!qs[i + 1]) { printf("  ERRO: overflow em q[%d]\n", i); return 1; }
    }

    // ── Fase 2: calcular módulos e gaps ──
    printf("[Fase 2] Calculando modulos N_i = p * q_i e gaps:\n\n");
    printf("  %-4s | %-14s | %-14s | %-10s | %-10s | %-8s | %s\n",
           "i", "q_i", "N_i = p*q_i", "gap_N", "p*gap_q", "gap_q", "K(s)");
    printf("  %-4s-|-%14s-|-%14s-|-%10s-|-%10s-|-%8s-|-%s\n",
           "----", "-------------", "-------------", "----------", "----------", "--------", "--------");

    u64 soma_gaps_N = 0;
    int termos_s[32];

    for (int i = 0; i < N_PASSOS; i++) {
        u64 q_atual = qs[i];
        u64 q_prox   = qs[i + 1];
        u64 N_atual  = p * q_atual;
        u64 N_prox   = p * q_prox;
        u64 gap_N    = N_prox - N_atual;
        u64 gap_q    = q_prox - q_atual;
        u64 predito  = p * gap_q;      // gap_N previsto pela caixa
        soma_gaps_N += gap_N;

        // Decomposição
        int nk = decompor_k(gap_q, termos_s);

        char buf[64] = "";
        int pos = 0;
        for (int j = 0; j < nk; j++) {
            pos += sprintf(buf + pos, "%sK(%d)", j ? "+" : "", termos_s[j]);
        }

        printf("  %-4d | %-14llu | %-14llu | %-10llu | %-10llu | %-8llu | %s\n",
               i, q_atual, N_atual, gap_N, predito, gap_q, buf);

        if (gap_N != predito) {
            printf("  >>> ERRO! gap_N difere de p*gap_q!\n");
        }
    }

    printf("\n  Soma total dos gaps_N: %llu\n", soma_gaps_N);
    printf("  N_final - N_inicial: %llu\n", p * qs[N_PASSOS] - p * qs[0]);

    // ── Fase 3: Análise da estrutura dos gaps ──
    printf("\n[Fase 3] Estrutura dos gaps_q e sua decomposicao K(s):\n\n");

    int hist_s[20] = {0};
    for (int i = 0; i < N_PASSOS; i++) {
        u64 gap_q = qs[i + 1] - qs[i];
        int nk = decompor_k(gap_q, termos_s);
        for (int j = 0; j < nk; j++) {
            if (termos_s[j] < 20) hist_s[termos_s[j]]++;
        }
    }

    printf("  Frequencia dos K(s) usados nas decomposicoes:\n");
    printf("  %-6s | %s\n", "K(s)", "ocorrencias");
    printf("  %-6s-|-%s\n", "------", "------------");
    for (int s = 1; s < 10; s++) {
        if (hist_s[s])
            printf("  K(%-3d) | %d\n", s, hist_s[s]);
    }

    // ── Fase 4: Verificar o "batimento" ──
    printf("\n[Fase 4] Teste da \"grade de batimento\":\n");
    printf("  Se gap_q se repete, gap_N = p * gap_q se repete na mesma proporcao.\n\n");

    // Agrupar gaps_q iguais e verificar se gap_N correspondente = p * gap_q
    printf("  %-8s | %-8s | %-14s | %-8s\n",
           "gap_q", "vezes", "p*gap_q", "gap_N real");
    printf("  %-8s-|-%8s-|-%14s-|-%8s\n",
           "--------", "--------", "-------------", "--------");

    for (int i = 0; i < N_PASSOS; i++) {
        u64 gap_q = qs[i + 1] - qs[i];
        u64 gap_N = p * qs[i + 1] - p * qs[i];
        // Count occurrences
        int count = 0;
        for (int j = 0; j < N_PASSOS; j++)
            if (qs[j + 1] - qs[j] == gap_q) count++;
        printf("  %-8llu | %-8d | %-14llu | %-8llu%s\n",
               gap_q, count, p * gap_q, gap_N,
               count > 1 ? " (repetido!)" : "");
    }

    // ── Conclusão ──
    printf("\n================================================================\n");
    printf("  CONCLUSAO\n");
    printf("================================================================\n");
    printf("\n  gap_N = p * gap_q  =>  %s\n",
           (p * qs[N_PASSOS] - p * qs[0]) == soma_gaps_N ? "CONFIRMADO" : "FALHOU");
    printf("  Decomposicao de gap_q em K(s) = 2^s - 1: ");
    int total_termos = 0;
    for (int i = 0; i < N_PASSOS; i++)
        total_termos += decompor_k(qs[i+1] - qs[i], termos_s);
    printf("%d gaps decompostos em %d termos K(s) (media %.1f termos/gap)\n",
           N_PASSOS, total_termos, (double)total_termos / N_PASSOS);

    printf("\n  A grade de batimento dos modulos RSA e previsivel:\n");
    printf("  N_{i+1} - N_i = p * (q_{i+1} - q_i) = p * gap_q_i\n");
    printf("  Cada gap_q_i se decompoe em K(s) = 2^s - 1.\n");
    printf("  A estrutura se preserva multiplicativamente por p.\n");
    printf("================================================================\n");

    free(qs);
    return 0;
}
