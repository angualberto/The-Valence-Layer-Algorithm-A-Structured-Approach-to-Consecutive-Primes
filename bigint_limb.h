// bigint_limb.h — Aritmética multiprecisão por concatenação de u64 limbs
#ifndef BIGINT_LIMB_H
#define BIGINT_LIMB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t limb_t;
typedef __int128 dlimb_t;  // double-limb para carry

#define LIMB_BITS 64
#define LIMB_MAX  (~0ULL)

// Estrutura: número = limbs[0] + limbs[1]*2^64 + limbs[2]*2^128 + ...
typedef struct {
    int n;          // número de limbs alocados
    int used;       // limbs realmente usados (used <= n)
    limb_t* limbs;  // little-endian: limbs[0] = parte baixa
} BigInt;

// ─── Alocação ──────────────────────────────────────────────────────────
static BigInt* bi_alloc(int n_limbs) {
    BigInt* a = (BigInt*)malloc(sizeof(BigInt));
    if (!a) return 0;
    a->limbs = (limb_t*)calloc(n_limbs, sizeof(limb_t));
    if (!a->limbs) { free(a); return 0; }
    a->n = n_limbs;
    a->used = 0;
    return a;
}
static void bi_free(BigInt* a) {
    if (a) { free(a->limbs); free(a); }
}
static void bi_trim(BigInt* a) {
    while (a->used > 0 && a->limbs[a->used-1] == 0) a->used--;
}
static void bi_zero(BigInt* a) {
    memset(a->limbs, 0, a->n * sizeof(limb_t));
    a->used = 0;
}

// ─── Inicialização a partir de u64 ─────────────────────────────────────
static void bi_from_u64(BigInt* a, uint64_t v) {
    bi_zero(a);
    a->limbs[0] = v;
    a->used = v ? 1 : 0;
}
static void bi_from_u64_limbs(BigInt* a, uint64_t v) {
    bi_zero(a);
    a->limbs[0] = v & 0xFFFFFFFF;
    a->limbs[1] = v >> 32;
    a->used = a->limbs[1] ? 2 : (a->limbs[0] ? 1 : 0);
}

// ─── Cópia ─────────────────────────────────────────────────────────────
static void bi_copy(BigInt* dst, const BigInt* src) {
    bi_zero(dst);
    int m = src->used < dst->n ? src->used : dst->n;
    memcpy(dst->limbs, src->limbs, m * sizeof(limb_t));
    dst->used = m;
}

// ─── Comparação: retorna -1, 0, 1 ─────────────────────────────────────
static int bi_cmp(const BigInt* a, const BigInt* b) {
    if (a->used != b->used) return a->used < b->used ? -1 : 1;
    for (int i = a->used - 1; i >= 0; i--) {
        if (a->limbs[i] != b->limbs[i])
            return a->limbs[i] < b->limbs[i] ? -1 : 1;
    }
    return 0;
}

// ─── Soma: c = a + b ──────────────────────────────────────────────────
static void bi_add(BigInt* c, const BigInt* a, const BigInt* b) {
    bi_zero(c);
    int max_used = (a->used > b->used ? a->used : b->used) + 1;
    if (max_used > c->n) max_used = c->n;
    limb_t carry = 0;
    int i;
    for (i = 0; i < max_used - 1; i++) {
        limb_t ai = i < a->used ? a->limbs[i] : 0;
        limb_t bi_ = i < b->used ? b->limbs[i] : 0;
        dlimb_t s = (dlimb_t)ai + bi_ + carry;
        c->limbs[i] = (limb_t)s;
        carry = (limb_t)(s >> LIMB_BITS);
    }
    c->limbs[i] = carry;
    c->used = max_used;
    bi_trim(c);
}

// ─── Subtração: c = a - b (a >= b) ─────────────────────────────────────
static void bi_sub(BigInt* c, const BigInt* a, const BigInt* b) {
    bi_zero(c);
    int max_used = a->used > b->used ? a->used : b->used;
    if (max_used > c->n) max_used = c->n;
    limb_t borrow = 0;
    for (int i = 0; i < max_used; i++) {
        limb_t ai = i < a->used ? a->limbs[i] : 0;
        limb_t bi_ = i < b->used ? b->limbs[i] : 0;
        dlimb_t diff = (dlimb_t)ai - bi_ - borrow;
        c->limbs[i] = (limb_t)diff;
        borrow = (diff >> LIMB_BITS) & 1;
    }
    c->used = max_used;
    bi_trim(c);
}

// ─── Multiplicação: c = a * b (b é u64 pequeno) ─────────────────────────
static void bi_mul_u64(BigInt* c, const BigInt* a, limb_t b) {
    bi_zero(c);
    if (b == 0) return;
    limb_t carry = 0;
    for (int i = 0; i < a->used && i < c->n - 1; i++) {
        dlimb_t prod = (dlimb_t)a->limbs[i] * b + carry;
        c->limbs[i] = (limb_t)prod;
        carry = (limb_t)(prod >> LIMB_BITS);
    }
    if (a->used < c->n) c->limbs[a->used] = carry;
    c->used = a->used + 1;
    bi_trim(c);
}

// ─── Divisão por u64: quoc = a / b, resto = a % b (b pequeno) ──────────
static limb_t bi_div_u64(BigInt* quoc, const BigInt* a, limb_t b) {
    bi_zero(quoc);
    if (b == 0) return LIMB_MAX;
    dlimb_t rem = 0;
    for (int i = a->used - 1; i >= 0; i--) {
        rem = (rem << LIMB_BITS) | a->limbs[i];
        if (i < quoc->n) {
            quoc->limbs[i] = (limb_t)(rem / b);
            rem %= b;
        }
    }
    quoc->used = a->used;
    bi_trim(quoc);
    return (limb_t)rem;
}

// ─── Divisibilidade: retorna 1 se a % b == 0 ────────────────────────────
static int bi_divisivel_u64(const BigInt* a, limb_t b) {
    if (b == 0) return 0;
    dlimb_t rem = 0;
    for (int i = a->used - 1; i >= 0; i--)
        rem = ((rem << LIMB_BITS) | a->limbs[i]) % b;
    return rem == 0;
}

// ─── Print ─────────────────────────────────────────────────────────────
static void bi_print(const BigInt* a) {
    if (a->used == 0) { printf("0"); return; }
    printf("0x");
    for (int i = a->used - 1; i >= 0; i--)
        printf("%016llx", a->limbs[i]);
}

// ─── Gera N = p * q (multi-precisão) ──────────────────────────────────
// p e q são u64, resultado em BigInt
static void bi_mul_u64_u64(BigInt* c, uint64_t p, uint64_t q) {
    bi_zero(c);
    // Decompor p e q em 32-bit halves
    uint64_t pl = p & 0xFFFFFFFF, ph = p >> 32;
    uint64_t ql = q & 0xFFFFFFFF, qh = q >> 32;
    // Produtos parciais
    uint64_t ll = (uint64_t)((dlimb_t)pl * ql);
    uint64_t lh = (uint64_t)((dlimb_t)pl * qh);
    uint64_t hl = (uint64_t)((dlimb_t)ph * ql);
    uint64_t hh = (uint64_t)((dlimb_t)ph * qh);
    // Montar no array de limbs
    uint64_t limbs[4] = {0};
    dlimb_t soma;
    soma = (dlimb_t)ll;
    limbs[0] = (uint64_t)soma;
    uint64_t carry = (uint64_t)(soma >> 32);  // carry de 32 bits

    soma = (dlimb_t)(ll >> 32) + (lh & 0xFFFFFFFF) + (hl & 0xFFFFFFFF) + carry;
    limbs[1] = ((uint64_t)(soma >> 32) << 32) | (uint64_t)soma;
    carry = (uint64_t)(soma >> 32);

    // Na verdade este método está errado. Vamos usar abordagem direta:
    bi_zero(c);
    dlimb_t p128 = (dlimb_t)p;
    dlimb_t q128 = (dlimb_t)q;
    dlimb_t prod = p128 * q128;
    c->limbs[0] = (uint64_t)prod;
    c->limbs[1] = (uint64_t)(prod >> 64);
    c->used = c->limbs[1] ? 2 : 1;
}

#endif
