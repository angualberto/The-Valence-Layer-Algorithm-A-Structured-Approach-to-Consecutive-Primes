#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

using namespace std;

// Template: extrai subchave de w bits usando propagacao de carry
template<typename T>
T extrair_subchave(const T& secreta, int w) {
    T reconstruida = 0;
    for (int bit = 0; bit < w; bit++) {
        T A = (bit == 0) ? 0 : ((T)1 << bit) - 1;
        T saida_real = A + secreta;
        T saida_teste = A + reconstruida;
        if ((saida_real & ((T)1 << bit)) != (saida_teste & ((T)1 << bit))) {
            reconstruida |= ((T)1 << bit);
        }
    }
    return reconstruida;
}

template<typename T>
bool testar(const T& secreta, int w, const char* nome) {
    T extraida = extrair_subchave(secreta, w);
    bool ok = (extraida == secreta);
    cout << "  " << nome << " secreta=0x" << hex << setw(w/4) << setfill('0') << secreta
         << " extraida=0x" << setw(w/4) << setfill('0') << extraida
         << " [" << (ok ? "OK" : "FALHA") << "]" << endl;
    return ok;
}

int main() {
    cout << "============================================================\n";
    cout << "   ATAQUE CARRY — TESTE EM MULTIPLOS TAMANHOS E CHAVES      \n";
    cout << "============================================================\n\n";

    int total = 0, acertos = 0;

    // --- w = 16 bits ---
    cout << "--- w = 16 bits (16 somas cada) ---\n";
    uint16_t chaves16[] = {0x0000, 0xFFFF, 0x1234, 0xABCD, 0x8000, 0x7FFF, 0xAAAA, 0x5555};
    for (auto k : chaves16) {
        total++; acertos += testar(k, 16, "S[]");
    }

    // --- w = 32 bits (RC5-32) ---
    cout << "\n--- w = 32 bits (32 somas cada) ---\n";
    uint32_t chaves32[] = {
        0x00000000, 0xFFFFFFFF, 0x5A3C96E7, 0x9E3779B9, 0xB7E15163,
        0x12345678, 0x87654321, 0xDEADBEEF, 0xCAFEBABE, 0x0F0F0F0F
    };
    for (auto k : chaves32) {
        total++; acertos += testar(k, 32, "S[]");
    }

    // --- w = 64 bits (RC5-64) ---
    cout << "\n--- w = 64 bits (64 somas cada) ---\n";
    uint64_t chaves64[] = {
        0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL,
        0x5A3C96E79E3779B9ULL, 0xB7E151630F0F0F0FULL,
        0x12345678ABCDEF01ULL, 0xDEADBEEFCAFEBABEULL
    };
    for (auto k : chaves64) {
        total++; acertos += testar(k, 64, "S[]");
    }

    cout << "\n--- Simulacao de 26 subchaves do RC5-32/12/16 (key schedule) ---\n";
    uint32_t S[26];
    srand(42);
    for (int i = 0; i < 26; i++) S[i] = (uint32_t)rand() ^ ((uint32_t)rand() << 16);

    int sub_ok = 0;
    for (int i = 0; i < 26; i++) {
        uint32_t extraida = extrair_subchave(S[i], 32);
        bool ok = (extraida == S[i]);
        sub_ok += ok;
        cout << "  S[" << dec << setw(2) << i << "] 0x" << hex << setw(8) << setfill('0') << S[i]
             << " -> 0x" << setw(8) << setfill('0') << extraida
             << " [" << (ok ? "OK" : "FALHA") << "]" << endl;
    }
    total += 26; acertos += sub_ok;

    cout << "\n============================================================\n";
    cout << " RESULTADO FINAL: " << acertos << "/" << total << " testes OK\n";
    cout << " Custo por subchave: w somas (w = 16, 32, 64 bits)\n";
    cout << "============================================================\n";

    return (acertos == total) ? 0 : 1;
}
