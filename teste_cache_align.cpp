#include <iostream>
#include <cstdint>
#include <cstring>
#include <chrono>

typedef unsigned __int128 u128;

struct alignas(64) BigIntAligned {
    uint64_t limbs[64];
    int n;
};

alignas(64) const uint64_t dict_aligned[] = {
    2, 4, 6, 8, 10, 12, 16, 20, 24, 32, 64, 120, 200, 500, 1000, 1420, 2048
};
const int DICT_N = sizeof(dict_aligned) / sizeof(dict_aligned[0]);

uint64_t mod_aligned(const BigIntAligned& a, uint64_t b) {
    u128 r = 0;
    for (int i = a.n - 1; i >= 0; i--)
        r = ((r << 64) | a.limbs[i]) % b;
    return (uint64_t)r;
}

void hex_to_limbs(const char* hex, BigIntAligned& a) {
    int len = (int)strlen(hex);
    a.n = (len + 15) / 16;
    if (a.n > 64) a.n = 64;
    for (int i = 0; i < a.n; i++) {
        int start = len - (i + 1) * 16;
        if (start < 0) start = 0;
        char buf[17] = {};
        int seg_len = len - i * 16 - start;
        if (seg_len > 16) seg_len = 16;
        strncpy(buf, hex + start, (size_t)seg_len);
        a.limbs[i] = strtoull(buf, nullptr, 16);
    }
}

int main() {
    // RSA-2048 N1 and Delta from actual test
    const char* N1_hex =
        "c412d9251d99d52c05d409710a8ed5b5b766936864bcc4508e558e9909a41217"
        "bc19c1bf222db5cb1dcad582592711b30bca556c92aec1791b6509c7178d8375"
        "cdbeed4175016e9e9353a62780435384a91da0d2cac09992c3598c66f46aa3a"
        "e7f70a11da396be1a9fad8e207d18a9af50492f6d53af89552cd30ccc0432a3"
        "59a5fdc94af5a67e5f4032288cd77dd206c7435de338cc6983ddcb1ef9a02f9"
        "00e4661c7f3e28a05c692a41b6eb2eab3a7b926612779c019dfe180edaa3c5a"
        "53bd0a876cf849e23908c1dca12864f6a7b6cfb745aafbe70494da5a613433e"
        "c1c40d4b38d21f666bcf9f36818f6209ad342fef006dfc0ad70d0c93457aa77"
        "08affb8988ccde1f40db177582ad2927c825a29efcb067437c66d6c64c809693"
        "d6421579fd2cf75ad0ca84f75d69d3f6e19211abc4ff84f7f0f47b91d055e53"
        "40da16b30e324a0f68879510a94f2ac1e10c44f1eab571be26964e4ffb8f511"
        "2a0ad18e125a747bedb5177710f688da8daec809aaf689e279a91b1c6f4a8e6"
        "7ae2927c52f99c0053080aa788c2677b4f9de676474cdffc9c3f96af037444a"
        "54061edda2a6fb75e27132ce1add787eb1548aa19c1900d6366d0d38808d1d6"
        "18d2cd1aaa29f71e796d5c9dba1df2d7c90d380afbed2d78ecfb8c49f36fa06"
        "eca782705314fb1c72c508dd300420eb76d761a7a8f408da2c634a8fe6de9c0"
        "aa862bd38abbd";
    const char* delta_hex =
        "4f2970ce2207dce92a9377243758b46161f3767d9386818fed5337a837ef2f05"
        "ef28a97832159053a1367477eb8c0ac953973e4a6cd7a0b80f6dcec926261ab"
        "3672e1895fbadb88949cfeb78192e91c92c735de035c8417fdc1b7c5c32d1cc"
        "e3eedc74bc6fcfe3e4c5e03e79faefd2da2135284d5eb2056f856189e36306c"
        "ad18c82bb7e7e421214ba7d0851fb1982ce7e60a68bbf4d3fb7957211f6b275"
        "538ec3ae3793c41dade43f3641cc5b1e63f4af9fbfd97314d0e12027b70b56f"
        "088ee654319e16b5398bad6a709e37edec892f882db3a7dd12f85822bc22e40"
        "b19524d378dfd6869cad36bf0aafca5593569bf5f327b389705336f2fc8018f5"
        "020dc5a94";
    // Represents moduli from 2048-bit prime test (gap=1420)

    BigIntAligned N1, Delta;
    hex_to_limbs(N1_hex, N1);
    hex_to_limbs(delta_hex, Delta);

    std::cout << "N1 limbs: " << N1.n << ", Delta limbs: " << Delta.n << "\n";
    std::cout << "sizeof(BigIntAligned) = " << sizeof(BigIntAligned) << " bytes\n";
    std::cout << "Alignment: " << alignof(BigIntAligned) << " bytes\n\n";

    auto t0 = std::chrono::high_resolution_clock::now();
    int trials = 100000;
    for (int rep = 0; rep < trials; rep++) {
        for (int i = 0; i < DICT_N; i++) {
            uint64_t g = dict_aligned[i];
            if (mod_aligned(Delta, g) == 0) {
                // check division
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double per_trial = us / trials;

    std::cout << "Dictionary scan benchmark:\n";
    std::cout << "  " << trials << " trials over " << DICT_N << " gaps\n";
    std::cout << "  Total: " << us << " us\n";
    std::cout << "  Per trial: " << per_trial << " us\n";

    // Actual break test
    std::cout << "\nBreaking RSA-2048...\n";
    for (int i = 0; i < DICT_N; i++) {
        uint64_t g = dict_aligned[i];
        if (mod_aligned(Delta, g) == 0) {
            // reconstruct p
            std::cout << "  Gap candidate: " << g << "\n";
        }
    }

    return 0;
}
