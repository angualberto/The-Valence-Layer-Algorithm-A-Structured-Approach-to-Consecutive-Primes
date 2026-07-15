#include <stdio.h>
#include <math.h>
#include <stdint.h>
typedef uint64_t u64;

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

int main() {
    u64 N = 19999851000029ULL;
    u64 lim = (u64)sqrt((double)N);
    printf("N = %llu, sqrt(N) = %llu\n", N, lim);
    printf("is_prime(3) = %d\n", is_prime_mr(3));
    printf("is_prime(4) = %d\n", is_prime_mr(4));
    printf("is_prime(5) = %d\n", is_prime_mr(5));
    printf("is_prime(3999971) = %d\n", is_prime_mr(3999971));
    printf("is_prime(4999999) = %d\n", is_prime_mr(4999999));

    int count = 0;
    for (u64 c = 3; c <= lim; c += 2) {
        count++;
        if (N % c == 0) {
            printf("Factor found: %llu (test %d)\n", c, count);
            break;
        }
    }
    printf("+2 stepping: tested %d candidates\n", count);
    return 0;
}
