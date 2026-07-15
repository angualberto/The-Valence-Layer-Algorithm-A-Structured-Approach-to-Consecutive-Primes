#include <iostream>
#include <iomanip>
#include <cstdint>

using WORD = uint32_t;

int main() {
    const WORD S2_secreta = 0x5A3C96E7;

    std::cout << "========================================================\n";
    std::cout << "    ATAQUE REVERSO POR PROPAGACAO DE CARRY (ODOMETRO)   \n";
    std::cout << "========================================================\n";

    WORD S2_reconstruida = 0;

    for (int bit = 0; bit < 32; bit++) {
        WORD teste_A = (1ULL << bit) - 1;

        WORD saida_real = (teste_A + S2_secreta);
        WORD saida_com_bit_zero = (teste_A + S2_reconstruida);

        if ((saida_real & (1ULL << bit)) != (saida_com_bit_zero & (1ULL << bit))) {
            S2_reconstruida |= (1ULL << bit);
        }
    }

    std::cout << " [CHAVE] Secreta Real        : 0x" << std::hex << S2_secreta << "\n";
    std::cout << " [CHAVE] Extraida pelo Carry : 0x" << std::hex << S2_reconstruida << "\n\n";

    if (S2_secreta == S2_reconstruida) {
        std::cout << " [OK] SUCESSO: Chave extraida sem forca bruta (32 somas)!\n";
    } else {
        std::cout << " [FALHA] Correlacao incorreta.\n";
    }
    std::cout << "========================================================\n";

    return 0;
}
