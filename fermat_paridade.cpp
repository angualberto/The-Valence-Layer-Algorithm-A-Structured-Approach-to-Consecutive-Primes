#include <iostream>
#include <chrono>
#include <cmath>
#include <iomanip>

using u64 = uint64_t;

// Raiz quadrada inteira exata (Newton, sem ponto flutuante para u64)
static inline u64 isqrt(u64 n) {
    if (n == 0) return 0;
    u64 x = n;
    u64 y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

static inline bool eh_quadrado_perfeito(u64 n, u64& raiz) {
    // Pré-filtro rápido: quadrados perfeitos mod 16 são apenas {0,1,4,9}
    unsigned m = n & 15;
    if (m > 9 || (m != 0 && m != 1 && m != 4 && m != 9)) return false;
    raiz = isqrt(n);
    return raiz * raiz == n;
}

int main() {
    const u64 p_real = 3999971;
    const u64 q_real = 4999999;
    const u64 N = p_real * q_real;

    std::cout << "========================================================\n";
    std::cout << "  FATORADOR GEOMETRICO POR PARIDADE\n";
    std::cout << "========================================================\n";
    std::cout << " N = " << N << "\n\n";

    u64 x = (u64)std::ceil(std::sqrt((double)N));
    u64 resto_4 = N % 4;

    std::cout << " sqrt(N) = " << x-1 << "." << int((sqrt((double)N) - (x-1))*100) << "\n";
    std::cout << " x_inicial = " << x << "\n";
    std::cout << " N mod 4 = " << resto_4 << "\n\n";

    if (resto_4 == 1) {
        std::cout << " N=1 mod 4  =>  x DEVE ser IMPAR\n";
        if (x % 2 == 0) x++;
    } else {
        std::cout << " N=3 mod 4  =>  x DEVE ser PAR\n";
        if (x % 2 != 0) x++;
    }
    std::cout << " x ajustado = " << x << "\n";
    std::cout << " x real (p+q)/2 = " << (p_real + q_real) / 2 << "\n";
    std::cout << " iteracoes esperadas = " << ((p_real+q_real)/2 - x) / 2 << "\n";
    std::cout << "--------------------------------------------------------\n";

    auto inicio = std::chrono::high_resolution_clock::now();
    u64 iteracoes = 0;
    u64 y = 0;
    bool achou = false;

    while (true) {
        iteracoes++;
        u64 x2 = x * x;
        if (x2 < N) { x += 2; continue; } // overflow check
        u64 y2 = x2 - N;
        if (eh_quadrado_perfeito(y2, y)) {
            achou = true;
            break;
        }
        x += 2;
        if (x > (N + 1) / 2) break;
    }

    auto fim = std::chrono::high_resolution_clock::now();
    double tempo = std::chrono::duration<double>(fim - inicio).count();

    if (achou) {
        u64 p_calc = x - y;
        u64 q_calc = x + y;
        std::cout << " SUCESSO!\n";
        std::cout << "  x = " << x << "\n";
        std::cout << "  y = " << y << "\n";
        std::cout << "  p = " << p_calc << "\n";
        std::cout << "  q = " << q_calc << "\n";
        std::cout << "  iteracoes = " << iteracoes << "\n";
        std::cout << "  tempo = " << std::fixed << std::setprecision(6) << tempo << " s\n\n";

        // Comparacao com trial division
        std::cout << "─── COMPARACAO ───\n";
        std::cout << " Trial division (+2): ~1.999.985 iteracoes (divisoes)\n";
        std::cout << " Fermat paridade:     " << iteracoes << " iteracoes (mult + sqrt)\n";
        std::cout << " Speedup teorico:     ~" << (1999985 / iteracoes) << "x menos iteracoes\n";
    } else {
        std::cout << " FALHA: fator nao encontrado\n";
    }
    std::cout << "========================================================\n";
    return 0;
}
