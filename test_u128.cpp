#include <stdio.h>
#include <stdint.h>
typedef uint64_t u64;
typedef unsigned __int128 u128;

int main() {
    printf("Test 1: u128 multiplication\n");
    u64 p = 140737488355333ULL;
    u64 q = 140737488355339ULL; // some prime > p
    u128 N = (u128)p * q;
    printf("  p=%llu q=%llu N=%llx%016llx\n", p, q, (u64)(N>>64), (u64)N);

    printf("Test 2: isqrt128\n");
    u128 sq = (u128)1 << 94;
    u128 r = sq;
    u128 x = r, y = (x + 1) / 2;
    int iter = 0;
    while (y < x) { x = y; y = (x + sq / x) / 2; iter++; if (iter > 100) break; }
    printf("  isqrt(2^94) = %llx%016llx (iter=%d)\n", (u64)(x>>64), (u64)x, iter);

    printf("Test 3: N %% c for small c\n");
    u64 c = 3;
    int count = 0;
    while (c < 1000 && N % c != 0) { c += 2; count++; }
    printf("  first divisor after %d tests: %llu\n", count, c);

    return 0;
}
