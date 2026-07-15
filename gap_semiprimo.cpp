#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>

using u64 = uint64_t;
using u128 = unsigned __int128;

// --- Máscaras de resíduos quadráticos ---
constexpr u64 mask_pow2(int bits) {
    u64 m = 0; u64 mod = 1ULL << bits;
    for (u64 i = 0; i < mod; i++) m |= 1ULL << ((i * i) & (mod - 1));
    return m;
}
constexpr u64 mask_mod(u64 mod) {
    u64 m = 0;
    for (u64 i = 0; i < mod; i++) m |= 1ULL << ((i * i) % mod);
    return m;
}

// --- Filtros ---
struct Filtro {
    u64 mod;
    u64 mask;
    bool pow2;
    bool passa(u64 y2) const {
        if (pow2) return (mask >> (y2 & (mod - 1))) & 1;
        else      return (mask >> (y2 % mod)) & 1;
    }
};

// Bateria ULTIMATE: mod 64 (bitwise) + mod 9 + mod 5 + mod 7
static const Filtro FILTROS[] = {
    {64, mask_pow2(6), true},
    {9,  mask_mod(9),  false},
    {5,  mask_mod(5),  false},
    {7,  mask_mod(7),  false},
};
static const int N_FILTROS = 4;

// --- Gap de semiprimo via Fermat bitwise ---
struct Resultado {
    u64 p, q, gap;
    u64 candidatos, rejeitados, chamadas_sqrt;
    double tempo;
};

Resultado fatora_com_gap(u64 N) {
    u64 x = static_cast<u64>(std::ceil(std::sqrt((long double)N)));

    // Alinhamento por paridade (N & 3)
    if ((N & 3) == 1) { if ((x & 1) == 0) x |= 1; }
    else              { if ((x & 1) != 0) x++; }

    u64 total = 0, rej = 0, y = 0;
    auto inicio = std::chrono::high_resolution_clock::now();

    while (true) {
        total++;
        u64 y2 = x * x - N;

        // Filtros em cascata
        bool valido = true;
        for (int i = 0; i < N_FILTROS; i++) {
            if (!FILTROS[i].passa(y2)) { rej++; valido = false; break; }
        }
        if (!valido) { x += 2; continue; }

        // Sqrt real
        u64 r = static_cast<u64>(std::sqrt((long double)y2));
        if (r * r == y2) { y = r; break; }
        x += 2;
    }

    auto fim = std::chrono::high_resolution_clock::now();

    u64 p = x - y, q = x + y;
    return {p, q, q - p, total, rej, total - rej,
            std::chrono::duration<double>(fim - inicio).count()};
}

// --- Ferramentaria Valence ---
static const int VALENCE[] = {1000,500,200,120,64,34,32,30,28,24,20,18,16,12,10,8,6,4,2};
static const int N_VALENCE = 19;

void decompoe_gap(u64 gap) {
    std::cout << "   gap = " << gap << "\n";
    std::cout << "   toolbox:";
    u64 resto = gap;
    for (int i = 0; i < N_VALENCE; i++) {
        int v = VALENCE[i];
        if (resto >= v) {
            int qtd = resto / v;
            resto -= qtd * v;
            std::cout << " " << qtd << "\u00D7" << v;
        }
    }
    std::cout << "\n";
    if (resto > 0) std::cout << "   resto (impar): " << resto << "\n";
    (void)resto;
}

// ============================================================

int main(int argc, char** argv) {
    std::cout << "============================================================\n";
    std::cout << "  GAP DE SEMIPRIMO N = p*q  (Filtros Bitwise + Valence)\n";
    std::cout << "============================================================\n\n";

    // Se N for passado como argumento, usa-o; senao, teste fixo
    u64 N;
    if (argc >= 2) {
        N = strtoull(argv[1], nullptr, 10);
    } else {
        // Testes com produtos de 2 primos conhecidos
        struct Teste { u64 p, q; };
        Teste testes[] = {
            {3999971ULL, 4999999ULL},
            {10007ULL, 10009ULL},
            {65521ULL, 65537ULL},
            {100003ULL, 100019ULL},
            {500009ULL, 500029ULL},
            {999983ULL, 1000003ULL},
            {3999971ULL, 4999999ULL},
            {10000019ULL, 10000079ULL},
        };
        int n_testes = sizeof(testes) / sizeof(testes[0]);

        for (int t = 0; t < n_testes; t++) {
            u64 p = testes[t].p, q = testes[t].q;
            u64 n = p * q;
            std::cout << "--- Teste " << (t+1) << " ------------------------------\n";
            std::cout << " N = " << n << " = " << p << " \u00D7 " << q << "\n";

            auto res = fatora_com_gap(n);

            std::cout << " p = " << res.p << "  q = " << res.q << "\n";
            std::cout << " gap = " << res.gap << "\n";

            u64 gap_esperado = q - p;
            std::cout << " gap esperado = " << gap_esperado;
            if (res.gap == gap_esperado) std::cout << " [OK]";
            else std::cout << " [FALHA: " << res.gap << "]";
            std::cout << "\n";

            double rej_pct = res.rejeitados * 100.0 / res.candidatos;
            std::cout << " candidatos: " << res.candidatos
                      << " | rejeitados: " << rej_pct << "%"
                      << " | sqrt()s: " << res.chamadas_sqrt
                      << " | tempo: " << std::fixed << std::setprecision(6)
                      << res.tempo << " s\n";

            std::cout << "\n";
        }

        std::cout << "--- Gap unico ------------------------------\n";
    }

    // Modo N via argumento
    if (argc >= 2) {
        std::cout << " N = " << N << "\n";
        auto res = fatora_com_gap(N);
        std::cout << " p = " << res.p << "  q = " << res.q
                  << "  gap = " << res.gap << "\n";
        double rej_pct = res.rejeitados * 100.0 / res.candidatos;
        std::cout << " candidatos: " << res.candidatos
                  << " | rejeitados: " << rej_pct << "%"
                  << " | sqrt()s: " << res.chamadas_sqrt
                  << " | tempo: " << std::setprecision(6) << res.tempo << " s\n";
        decompoe_gap(res.gap);
    }

    std::cout << "============================================================\n";
    return 0;
}
