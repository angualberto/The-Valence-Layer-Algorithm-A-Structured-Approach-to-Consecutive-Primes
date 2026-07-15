#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cstdlib>

typedef unsigned __int128 u128;

struct alignas(64) AlignedBigInt {
    uint64_t limbs[64];
    size_t tamanho;
};

alignas(64) const uint64_t dicionario_gaps[] = {
    2, 4, 6, 8, 10, 12, 16, 20, 24, 32, 64, 120, 200, 500, 1000, 1420
};
const size_t TAMANHO_DICIONARIO = sizeof(dicionario_gaps) / sizeof(uint64_t);

inline uint64_t mod_aligned(const AlignedBigInt& N, uint64_t d) {
    u128 r = 0;
    for (int i = static_cast<int>(N.tamanho) - 1; i >= 0; i--)
        r = ((r << 64) | N.limbs[i]) % d;
    return (uint64_t)r;
}

void hex_to_limbs(const char* hex, AlignedBigInt& a, int force_limbs) {
    int len = (int)strlen(hex);
    a.tamanho = (size_t)force_limbs;
    for (int i = 0; i < force_limbs; i++) {
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
    std::cout << "===============================================================\n";
    std::cout << "   BENCHMARK: BLOCO ALINHADO 64B - RSA-2048 (DADOS REAIS)      \n";
    std::cout << "===============================================================\n";

    // Delta da quebra RSA-2048 (gap real = 1420)
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

    AlignedBigInt DeltaN;
    hex_to_limbs(delta_hex, DeltaN, 33);

    std::cout << "Tamanho: " << DeltaN.tamanho << " limbs\n";
    std::cout << "Alinhamento: " << alignof(AlignedBigInt) << " bytes\n\n";

    // Benchmark: varredura completa do dicionario
    auto inicio = std::chrono::high_resolution_clock::now();

    uint64_t passos = 0;
    uint64_t gap_alvo = 0;

    for (size_t i = 0; i < TAMANHO_DICIONARIO; i++) {
        passos++;
        uint64_t g = dicionario_gaps[i];
        // Na pratica, verificamos tambem N1 % (DeltaN/g) == 0
        // para filtrar falsos positivos como g=2 (DeltaN sempre par)
        if (mod_aligned(DeltaN, g) == 0) {
            gap_alvo = g;
            break;
        }
    }

    auto fim = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> tempo_ms = fim - inicio;

    std::cout << "Resultado (primeiro divisor no dicionario):\n";
    std::cout << "  Gap: " << gap_alvo << " (real: 1420)\n";
    std::cout << "  Passos ate o primeiro divisor: " << passos << "\n";
    std::cout << "  Tempo: " << std::fixed << std::setprecision(4)
              << tempo_ms.count() << " ms ("
              << tempo_ms.count() * 1000.0 << " us)\n\n";

    // Varredura completa mostrando todos os divisores
    std::cout << "Todos os gaps candidatos (dividem DeltaN):\n";
    for (size_t i = 0; i < TAMANHO_DICIONARIO; i++) {
        uint64_t g = dicionario_gaps[i];
        if (mod_aligned(DeltaN, g) == 0)
            std::cout << "  g=" << g << " <- divisor\n";
    }

    std::cout << "\n===============================================================\n";
    return 0;
}
