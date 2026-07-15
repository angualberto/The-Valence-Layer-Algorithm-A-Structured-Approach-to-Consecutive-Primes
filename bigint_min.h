// bigint_min.h — BigInt mínimo (concatenação de limbs u64)
#ifndef BIGINT_MIN_H
#define BIGINT_MIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint64_t w64;
typedef __int128 w128;

#define LIMB_BITS 64
#define MAX_LIMBS 64

typedef struct { int n; w64 l[MAX_LIMBS]; } BI;

static void bi_zero(BI* a) { memset(a->l, 0, sizeof(a->l)); a->n = 0; }
static void bi_trim(BI* a) { while (a->n > 0 && a->l[a->n-1] == 0) a->n--; }
static int bi_is_zero(const BI* a) { return a->n == 0; }

static void bi_copy(BI* dst, const BI* src) {
    memcpy(dst->l, src->l, sizeof(w64) * src->n);
    dst->n = src->n;
}

static void bi_from_u64(BI* a, w64 v) { bi_zero(a); a->l[0] = v; a->n = v ? 1 : 0; }

// Converte bytes BIG-ENDIAN para BI (little-endian limbs)
// b[0] = byte mais significativo, b[len-1] = menos significativo
static void bi_from_be(const unsigned char* b, int len, BI* a) {
    bi_zero(a);
    int nl = (len + 7) / 8;
    if (nl > MAX_LIMBS) nl = MAX_LIMBS;
    for (int i = 0; i < nl; i++) {
        w64 v = 0;
        int start = len - (i+1)*8;  // início deste limb no array BE
        if (start < 0) start = 0;
        int end = len - i*8;         // fim (exclusivo)
        int bi = 0;
        for (int j = end-1; j >= start; j--) {
            v |= ((w64)b[j]) << (bi*8); bi++;
        }
        a->l[i] = v;
    }
    a->n = nl; bi_trim(a);
}
// Converte BI para bytes BIG-ENDIAN
static void bi_to_be(const BI* a, unsigned char* b, int len) {
    memset(b, 0, len);
    for (int i = 0; i < a->n && i*8 < len; i++) {
        w64 v = a->l[i];
        int start = len - (i+1)*8;
        if (start < 0) { // limb parcial no inicio
            int bytes = len - i*8;
            if (bytes > 8) bytes = 8;
            for (int j = 0; j < bytes; j++)
                b[j] = (unsigned char)(v >> ((bytes-1-j)*8));
        } else {
            for (int j = 0; j < 8; j++)
                b[start + j] = (unsigned char)(v >> ((7-j)*8));
        }
    }
}

static int bi_cmp(const BI* a, const BI* b) {
    if (a->n != b->n) return a->n < b->n ? -1 : 1;
    for (int i = a->n-1; i >= 0; i--)
        if (a->l[i] != b->l[i]) return a->l[i] < b->l[i] ? -1 : 1;
    return 0;
}

static void bi_sub(BI* c, const BI* a, const BI* b) {
    bi_zero(c);
    int mn = a->n > b->n ? a->n : b->n;
    w128 br = 0;
    for (int i = 0; i < mn; i++) {
        w128 av = i < a->n ? a->l[i] : 0;
        w128 bv = i < b->n ? b->l[i] : 0;
        w128 d = av - bv - br;
        c->l[i] = (w64)d; br = (d >> LIMB_BITS) & 1;
    }
    c->n = mn; bi_trim(c);
}

static w64 bi_mod_u64(const BI* a, w64 b) {
    if (!b) return 0;
    w128 r = 0;
    for (int i = a->n-1; i >= 0; i--) r = ((r << LIMB_BITS) | a->l[i]) % b;
    return (w64)r;
}

static w64 bi_div_u64(BI* q, const BI* a, w64 b) {
    bi_zero(q); if (!b) return 0;
    w128 r = 0;
    for (int i = a->n-1; i >= 0; i--) {
        r = (r << LIMB_BITS) | a->l[i];
        q->l[i] = (w64)(r / b); r %= b;
    }
    q->n = a->n; bi_trim(q);
    return (w64)r;
}

static void bi_shl(BI* c, const BI* a, int bits) {
    bi_zero(c);
    int ls = bits / LIMB_BITS, bs = bits % LIMB_BITS;
    if (a->n + ls >= MAX_LIMBS) return;
    w64 carry = 0;
    for (int i = 0; i < a->n; i++) {
        w128 v = ((w128)a->l[i] << bs) | carry;
        c->l[i+ls] = (w64)v; carry = (w64)(v >> LIMB_BITS);
    }
    if (carry && a->n+ls < MAX_LIMBS) c->l[a->n+ls] = carry;
    c->n = a->n + ls + (carry ? 1 : 0); bi_trim(c);
}

static void bi_mul_u64(BI* c, const BI* a, w64 b) {
    bi_zero(c); if (!b) return;
    w64 carry = 0;
    for (int i = 0; i < a->n && i < MAX_LIMBS-1; i++) {
        w128 p = (w128)a->l[i] * b + carry;
        c->l[i] = (w64)p; carry = (w64)(p >> LIMB_BITS);
    }
    if (a->n < MAX_LIMBS) c->l[a->n] = carry;
    c->n = a->n + 1; bi_trim(c);
}

// MSB position (0-indexed)
static int bi_msb(const BI* a) {
    if (a->n == 0) return -1;
    w64 top = a->l[a->n - 1];
    int pos = (a->n - 1) * LIMB_BITS;
    while (top) { top >>= 1; pos++; }
    return pos;
}

// a % b (b bigint) — divisão binária longa
static void bi_mod(BI* r, const BI* a, const BI* b) {
    if (bi_cmp(a, b) < 0) { bi_copy(r, a); return; }
    BI tmp, sb, df;
    bi_copy(&tmp, a); bi_zero(&sb); bi_zero(&df);

    int msb_a = bi_msb(a);
    int msb_b = bi_msb(b);
    int shift = msb_a - msb_b;

    while (shift >= 0) {
        bi_shl(&sb, b, shift);
        if (bi_cmp(&sb, &tmp) <= 0) {
            bi_sub(&df, &tmp, &sb);
            bi_copy(&tmp, &df);
        }
        shift--;
    }
    bi_copy(r, &tmp);
}

static void bi_mul_u64_u64(BI* c, w64 p, w64 q) {
    bi_zero(c);
    w128 prod = (w128)p * q;
    c->l[0] = (w64)prod; c->l[1] = (w64)(prod >> 64);
    c->n = c->l[1] ? 2 : 1;
}

static void bi_hex(const BI* a) {
    if (!a->n) { printf("0"); return; }
    for (int i = a->n-1; i >= 0; i--) printf("%016llx", a->l[i]);
}

#endif
