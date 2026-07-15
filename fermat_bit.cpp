#include <iostream>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <array>
#include <cstring>

using u64 = uint64_t;
using u128 = unsigned __int128;

// --- Geração de máscaras ---

constexpr u64 mask_pow2(int bits) {
    u64 m = 0;
    u64 mod = 1ULL << bits;
    for (u64 i = 0; i < mod; i++)
        m |= 1ULL << ((i * i) & (mod - 1));
    return m;
}

constexpr u64 mask_mod64(u64 mod) {
    u64 m = 0;
    for (u64 i = 0; i < mod; i++)
        m |= 1ULL << ((i * i) % mod);
    return m;
}

constexpr u128 mask_mod128(u64 mod) {
    u128 m = 0;
    for (u64 i = 0; i < mod; i++)
        m |= (u128)1 << ((i * i) % mod);
    return m;
}

struct Filtro {
    u64 modulo;
    u128 mascara;
    bool potencia2;

    int conta_residuos() const {
        int c = 0;
        for (u64 i = 0; i < modulo; i++)
            if ((mascara >> i) & 1) c++;
        return c;
    }

    bool passa(u64 y2) const {
        if (potencia2)
            return (mascara >> (y2 & (modulo - 1))) & 1;
        else
            return (mascara >> (y2 % modulo)) & 1;
    }
};

struct Bateria {
    const char* nome;
    std::array<Filtro, 5> filtros;
    int n_filtros;

    bool passa(u64 y2, int& idx_rej) const {
        for (int i = 0; i < n_filtros; i++) {
            if (!filtros[i].passa(y2)) {
                idx_rej = i;
                return false;
            }
        }
        idx_rej = -1;
        return true;
    }

    void mostra() const {
        std::cout << "  " << nome << ": ";
        double taxa = 1.0;
        for (int i = 0; i < n_filtros; i++) {
            auto& f = filtros[i];
            double pct = f.conta_residuos() * 100.0 / f.modulo;
            std::cout << "mod " << f.modulo << " (" << std::setprecision(2) << pct << "%)";
            if (i < n_filtros - 1) std::cout << " + ";
            taxa *= f.conta_residuos() / (double)f.modulo;
        }
        std::cout << "\n    => Passagem: " << std::fixed << std::setprecision(2)
                  << (taxa * 100.0) << "% | Rejeição: " << (100.0 - taxa * 100.0) << "%\n";
    }
};

struct Resultado {
    u64 total, rejeitados;
    u64 rej_filtro[5]{};
    u64 p, q;
    double tempo;
    bool ok;
};

Resultado fatora(u64 N, const Bateria& bat) {
    u64 x = static_cast<u64>(std::ceil(std::sqrt(N)));
    if ((N & 3) == 1) { if ((x & 1) == 0) x |= 1; }
    else              { if ((x & 1) != 0) x++; }

    u64 total = 0, rejeitados = 0, rej_f[5]{}, y = 0;
    auto inicio = std::chrono::high_resolution_clock::now();

    while (true) {
        total++;
        u64 y2 = x * x - N;

        int idx;
        if (!bat.passa(y2, idx)) {
            rejeitados++;
            if (idx >= 0) rej_f[idx]++;
            x += 2;
            continue;
        }

        u64 r = static_cast<u64>(std::sqrt(y2));
        if (r * r == y2) { y = r; break; }
        x += 2;
    }

    auto fim = std::chrono::high_resolution_clock::now();
    return {total, rejeitados, {rej_f[0], rej_f[1], rej_f[2], rej_f[3], rej_f[4]},
            x - y, x + y, std::chrono::duration<double>(fim - inicio).count(), true};
}

// ============================================================

int main() {
    const u64 N = (u64)3999971 * 4999999;

    // Baterias
    Bateria b1{"Mod 16 (original)"};
    b1.filtros[0] = {16, mask_pow2(4), true};
    b1.n_filtros = 1;

    Bateria b2{"Mod 64 (bitwise puro)"};
    b2.filtros[0] = {64, mask_pow2(6), true};
    b2.n_filtros = 1;

    Bateria b3{"Mod 16 + Mod 9"};
    b3.filtros[0] = {16, mask_pow2(4), true};
    b3.filtros[1] = {9,  mask_mod64(9), false};
    b3.n_filtros = 2;

    Bateria b4{"Mod 16 + Mod 9 + Mod 7"};
    b4.filtros[0] = {16, mask_pow2(4), true};
    b4.filtros[1] = {9,  mask_mod64(9), false};
    b4.filtros[2] = {7,  mask_mod64(7), false};
    b4.n_filtros = 3;

    Bateria b5{"Mod 64 + Mod 9 + Mod 5"};
    b5.filtros[0] = {64, mask_pow2(6), true};
    b5.filtros[1] = {9,  mask_mod64(9), false};
    b5.filtros[2] = {5,  mask_mod64(5), false};
    b5.n_filtros = 3;

    Bateria b6{"Mod 120 (128-bit mask)"};
    b6.filtros[0] = {120, mask_mod128(120), false};
    b6.n_filtros = 1;

    Bateria b7{"Mod 120 = CRT(8,3,5)"};
    b7.filtros[0] = {8,  mask_pow2(3), true};
    b7.filtros[1] = {3,  mask_mod64(3), false};
    b7.filtros[2] = {5,  mask_mod64(5), false};
    b7.n_filtros = 3;

    Bateria b8{"Mod 64 + Mod 9 + Mod 5 + Mod 7 (ULTIMATE)"};
    b8.filtros[0] = {64, mask_pow2(6), true};
    b8.filtros[1] = {9,  mask_mod64(9), false};
    b8.filtros[2] = {5,  mask_mod64(5), false};
    b8.filtros[3] = {7,  mask_mod64(7), false};
    b8.n_filtros = 4;

    std::array<Bateria*, 8> todas = {&b1, &b2, &b3, &b4, &b5, &b6, &b7, &b8};

    std::cout << "================================================================\n";
    std::cout << "  FILTROS BITWISE EMPILHADOS - FERMAT NO NIVEL DO SILICIO\n";
    std::cout << "================================================================\n";
    std::cout << " N = " << N << " = 3999971 × 4999999\n\n";
    std::cout << "--- Residuos teoricos ---\n";
    for (auto& b : todas) b->mostra();
    std::cout << "---------------------------------------------------------------\n\n";

    for (auto& b : todas) {
        auto r = fatora(N, *b);
        double rej = r.rejeitados * 100.0 / r.total;
        std::cout << "[" << b->nome << "]\n"
                  << "  Candidatos: " << r.total
                  << " | Rejeitados: " << r.rejeitados
                  << " (" << std::fixed << std::setprecision(2) << rej << "%)\n"
                  << "  sqrt()s   : " << (r.total - r.rejeitados)
                  << " | Tempo: " << std::setprecision(6) << r.tempo << " s\n";
        // Rejeição por estágio
        for (int i = 0; i < b->n_filtros; i++) {
            if (r.rej_filtro[i] > 0) {
                double pct = r.rej_filtro[i] * 100.0 / r.total;
                std::cout << "    ├─ mod " << b->filtros[i].modulo
                          << " rejeitou " << std::setprecision(2) << pct << "%\n";
            }
        }
        std::cout << "  p=" << r.p << " q=" << r.q << "\n";
        std::cout << "---------------------------------------------------------------\n";
    }

    return 0;
}
