#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <bitset>

const int w = 32;
const int r = 12;
const int b = 16;
const int t = 26;
const int BLOCO_BITS = 64;

using WORD = uint32_t;

const WORD P32 = 0xB7E15163;
const WORD Q32 = 0x9E3779B9;

WORD S[t];

inline WORD ROTL(WORD x, WORD y) {
    return ((x << (y & (w - 1))) | (x >> (w - (y & (w - 1)))));
}

inline WORD ROTR(WORD x, WORD y) {
    return ((x >> (y & (w - 1))) | (x << (w - (y & (w - 1)))));
}

void RC5_SETUP(const uint8_t* K) {
    const int u = w / 8;
    const int c = (b > 0) ? ((b + u - 1) / u) : 1;
    WORD L[4] = {0};
    for (int i = b - 1; i >= 0; i--)
        L[i / u] = (L[i / u] << 8) + K[i];
    S[0] = P32;
    for (int i = 1; i < t; i++)
        S[i] = S[i - 1] + Q32;
    WORD A = 0, B = 0;
    int i = 0, j = 0;
    int total_passos = 3 * ((t > c) ? t : c);
    for (int k = 0; k < total_passos; k++) {
        A = S[i] = ROTL(S[i] + (A + B), 3);
        B = L[j] = ROTL(L[j] + (A + B), (A + B));
        i = (i + 1) % t;
        j = (j + 1) % c;
    }
}

void RC5_ENCRYPT(const WORD* pt, WORD* ct) {
    WORD A = pt[0] + S[0];
    WORD B = pt[1] + S[1];
    for (int i = 1; i <= r; i++) {
        A = ROTL(A ^ B, B) + S[2 * i];
        B = ROTL(B ^ A, A) + S[2 * i + 1];
    }
    ct[0] = A;
    ct[1] = B;
}

// Contar bits diferentes entre dois blocos de 64 bits
int popcount64(WORD a0, WORD a1, WORD b0, WORD b1) {
    WORD d0 = a0 ^ b0;
    WORD d1 = a1 ^ b1;
    return std::bitset<32>(d0).count() + std::bitset<32>(d1).count();
}

int main() {
    const uint8_t chave[16] = {
        0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
        0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF
    };

    RC5_SETUP(chave);

    // Bloco base
    WORD base[2] = { 0xFEEDFACE, 0xDEADBEEF };
    WORD ct_base[2];
    RC5_ENCRYPT(base, ct_base);

    int total_diffs = 0;
    int min_diff = BLOCO_BITS + 1, max_diff = -1;
    int histograma[65] = {0};

    std::cout << "========================================================\n";
    std::cout << "     TESTE DE AVALANCHE - RC5-32/12/16                \n";
    std::cout << "========================================================\n";
    std::cout << " Bloco base: 0x" << std::hex << base[0] << " " << base[1] << "\n";
    std::cout << " Cifrado    : 0x" << ct_base[0] << " " << ct_base[1] << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << " Bit | Cifrado (bits invertidos)                    | Diff\n";
    std::cout << "--------------------------------------------------------\n";

    for (int bit = 0; bit < BLOCO_BITS; bit++) {
        WORD pt_mod[2] = { base[0], base[1] };

        // Inverter o bit 'bit' no plaintext
        if (bit < 32)
            pt_mod[0] ^= (1u << bit);
        else
            pt_mod[1] ^= (1u << (bit - 32));

        WORD ct_mod[2];
        RC5_ENCRYPT(pt_mod, ct_mod);

        int diff = popcount64(ct_base[0], ct_base[1], ct_mod[0], ct_mod[1]);
        total_diffs += diff;
        if (diff < min_diff) min_diff = diff;
        if (diff > max_diff) max_diff = diff;
        histograma[diff]++;

        std::cout << " " << std::dec << std::setw(2) << std::setfill('0') << bit
                  << " | 0x" << std::hex << std::setw(8) << std::setfill('0') << ct_mod[0]
                  << " " << std::setw(8) << ct_mod[1]
                  << " | " << std::dec << std::setw(2) << diff << "\n";
    }

    double media = total_diffs / (double)BLOCO_BITS;

    std::cout << "--------------------------------------------------------\n";
    std::cout << " ESTATISTICAS DE AVALANCHE (64 bits de bloco):\n";
    std::cout << "   Media de bits alterados : " << std::fixed << std::setprecision(2)
              << media << " / 64\n";
    std::cout << "   Minimo                  : " << min_diff << " bits\n";
    std::cout << "   Maximo                  : " << max_diff << " bits\n";
    std::cout << "   Ideal teorico           : 32 bits (50% do bloco)\n";
    std::cout << "   Desvio do ideal         : " << std::setprecision(2)
              << (media - 32.0) << " bits\n\n";

    std::cout << " HISTOGRAMA DE DISTRIBUICAO:\n";
    std::cout << "   Diffs | Ocorrencias | Barra\n";
    for (int d = 0; d <= BLOCO_BITS; d++) {
        if (histograma[d] > 0) {
            std::cout << "   " << std::setw(2) << d << "     | "
                      << std::setw(10) << histograma[d] << " | ";
            for (int k = 0; k < histograma[d]; k++) std::cout << "#";
            std::cout << "\n";
        }
    }

    std::cout << "--------------------------------------------------------\n";
    if (media > 28.0 && media < 36.0)
        std::cout << " [OK] PROPRIEDADE DE AVALANCHE CONFIRMADA\n";
    else
        std::cout << " [ALERTA] Avalanche fora da faixa esperada\n";
    std::cout << "========================================================\n";

    return 0;
}
