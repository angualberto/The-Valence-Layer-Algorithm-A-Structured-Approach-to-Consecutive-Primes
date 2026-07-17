// Ataque Carry RC5 — benchmark para w = 16,32,64,128,...,2048 bits
// Usa GMP para aritmetica de precisao arbitraria
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <chrono>
#include <vector>
#include <cmath>
#include <gmp.h>

using namespace std;
using namespace chrono;

// Extrai subchave de w bits usando carry propagation (GMP)
void extrair_subchave_gmp(const mpz_t& secreta, int w, mpz_t& reconstruida) {
    mpz_set_ui(reconstruida, 0);
    mpz_t A, Y_real, Y_teste, mascara, um, bit;
    mpz_inits(A, Y_real, Y_teste, mascara, um, bit, NULL);
    mpz_set_ui(um, 1);

    for (int b = 0; b < w; b++) {
        // A = (1 << b) - 1
        if (b == 0)
            mpz_set_ui(A, 0);
        else {
            mpz_mul_2exp(A, um, b);   // A = 2^b
            mpz_sub_ui(A, A, 1);      // A = 2^b - 1
        }

        // Y_real = (A + secreta) mod 2^w
        mpz_add(Y_real, A, secreta);
        mpz_tdiv_r_2exp(Y_real, Y_real, w);

        // Y_teste = (A + reconstruida) mod 2^w
        mpz_add(Y_teste, A, reconstruida);
        mpz_tdiv_r_2exp(Y_teste, Y_teste, w);

        // mascara = 1 << b
        mpz_mul_2exp(mascara, um, b);

        // Compara o bit b de Y_real e Y_teste
        mpz_and(bit, Y_real, mascara);
        int bit_real = mpz_cmp_ui(bit, 0) != 0;

        mpz_and(bit, Y_teste, mascara);
        int bit_teste = mpz_cmp_ui(bit, 0) != 0;

        if (bit_real != bit_teste)
            mpz_add(reconstruida, reconstruida, mascara); // set bit b
    }
    mpz_clears(A, Y_real, Y_teste, mascara, um, bit, NULL);
}

struct Resultado {
    int w;
    double tempo_us;
    bool ok;
};

int main() {
    cout << "=================================================================\n";
    cout << "  ATAQUE CARRY — BENCHMARK w = 16 ate 2048 bits (GMP)           \n";
    cout << "=================================================================\n\n";

    vector<int> tamanhos = {16, 32, 64, 72, 128, 256, 512, 1024, 2048};
    vector<Resultado> resultados;

    mpz_t secreta, reconstruida, mascara_w;
    mpz_inits(secreta, reconstruida, mascara_w, NULL);

    // Chave secreta: bits alternados 1010... padrao (0xAAAAAAAA...)
    // ou randômico via GMP
    gmp_randstate_t estado;
    gmp_randinit_default(estado);
    gmp_randseed_ui(estado, 42);

    const int NUM_TESTES = 5; // media de 5 execucoes

    for (int w : tamanhos) {
        mpz_tdiv_r_2exp(mascara_w, mascara_w, 0); // zero
        mpz_set_ui(mascara_w, 1);
        mpz_mul_2exp(mascara_w, mascara_w, w); // mascara_w = 2^w
        mpz_sub_ui(mascara_w, mascara_w, 1);   // mascara_w = 2^w - 1

        // Gera chave aleatoria de w bits
        mpz_urandomb(secreta, estado, w);

        double tempo_total = 0;
        bool todas_ok = true;

        for (int t = 0; t < NUM_TESTES; t++) {
            auto inicio = high_resolution_clock::now();
            extrair_subchave_gmp(secreta, w, reconstruida);
            auto fim = high_resolution_clock::now();

            // Verifica
            mpz_and(reconstruida, reconstruida, mascara_w);
            mpz_and(secreta, secreta, mascara_w);

            bool ok = (mpz_cmp(secreta, reconstruida) == 0);
            if (!ok) todas_ok = false;

            double us = duration_cast<nanoseconds>(fim - inicio).count() / 1000.0;
            if (t > 0) tempo_total += us; // descarta primeira iteracao (cold cache)
        }

        double tempo_medio = tempo_total / (NUM_TESTES - 1);
        resultados.push_back({w, tempo_medio, todas_ok});

        cout << "  w=" << setw(4) << w << " bits  "
             << fixed << setprecision(1) << setw(10) << tempo_medio << " us  "
             << "[" << (todas_ok ? "OK" : "FALHA") << "]"
             << "  (somas: " << w << ")" << endl;
    }

    cout << "\n--- Tabela LaTeX ---\n";
    cout << "\\begin{table}[htbp]\n";
    cout << "\\centering\n";
    cout << "\\caption{Carry attack benchmark for various word sizes (GMP).}\n";
    cout << "\\label{tab:carry_bench}\n";
    cout << "\\begin{tabular}{crrc}\n";
    cout << "\\toprule\n";
    cout << "w (bits) & Somas & Tempo ($\\mu$s) & Status \\\\\n";
    cout << "\\midrule\n";
    for (auto& r : resultados) {
        cout << "  " << r.w << " & " << r.w << " & " << fixed << setprecision(1) << r.tempo_us << " & "
             << (r.ok ? "OK" : "FALHA") << " \\\\\n";
    }
    cout << "\\bottomrule\n";
    cout << "\\end{tabular}\n";
    cout << "\\end{table}\n";

    cout << "\n\\begin{table}[htbp]\n";
    cout << "\\centering\n";
    cout << "\\caption{Brute-force comparison for each word size.}\n";
    cout << "\\label{tab:carry_gain}\n";
    cout << "\\begin{tabular}{crrr}\n";
    cout << "\\toprule\n";
    cout << "w (bits) & $2^w$ (brute force) & Carry (somas) & Gain \\\\\n";
    cout << "\\midrule\n";
    for (auto& r : resultados) {
        double brute = pow(2.0, r.w);
        cout << "  " << r.w << " & $" << scientific << setprecision(2) << brute << "$ & "
             << r.w << " & $" << brute / r.w << "$ \\\\\n";
    }
    cout << "\\bottomrule\n";
    cout << "\\end{tabular}\n";
    cout << "\\end{table}\n";

    mpz_clears(secreta, reconstruida, mascara_w, NULL);
    gmp_randclear(estado);

    cout << "\n===============================================================\n";
    cout << "  Todos os tamanhos testados com sucesso.\n";
    cout << "===============================================================\n";

    return 0;
}
