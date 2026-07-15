#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <string>

// Estrutura para facilitar a exibição dos testes de Fermat
struct FermatState {
    uint64_t N;
    uint64_t p;
    uint64_t q;
    FermatState(uint64_t n, uint64_t a, uint64_t b) : N(n), p(a), q(b) {}
    void analisar() {
        uint64_t resto = N & 3; // N % 4 via bitwise AND
        uint64_t x = (p + q) / 2;
        uint64_t y = (q - p) / 2;

        std::cout << std::setw(5) << N << " | "
                  << p << " x " << std::setw(2) << q << " | "
                  << std::setw(7) << resto << " | "
                  << std::setw(11) << ((x & 1) ? "IMPAR" : "PAR") << " | "
                  << std::setw(11) << ((y & 1) ? "IMPAR" : "PAR") << " | "
                  << "x = " << std::setw(2) << x << ", y = " << y << "\n";
    }
};

// Dicionário de valências pares otimizado para saltos rápidos
const std::vector<uint64_t> dicionario_gaps = {2, 4, 6, 8, 10, 12, 16, 20, 24, 32, 64, 120, 200, 500, 1000};

/**
 * @brief Executa o ataque de inversão de gap utilizando a paridade dupla
 */
void testarAtaquePorGap(uint64_t N1, uint64_t N2, std::string descricao) {
    std::cout << "\n[Ataque] " << descricao << "\n";
    std::cout << "  -> N1 = " << N1 << ", N2 = " << N2 << "\n";

    // Diferença física (sempre par para primos > 2)
    uint64_t DeltaN = (N2 > N1) ? (N2 - N1) : (N1 - N2);
    std::cout << "  -> DeltaN (Diferença) = " << DeltaN << " (Par: " << ((DeltaN % 2 == 0) ? "SIM" : "NAO") << ")\n";

    bool quebrado = false;
    uint64_t passos = 0;

    // O loop de busca utiliza exclusivamente o dicionário de valências pares
    for (uint64_t g : dicionario_gaps) {
        passos++;
        if (DeltaN % g == 0) {
            uint64_t fator_compartilhado = DeltaN / g;

            // Validação rápida: se divide um dos módulos, quebrou
            if (N1 % fator_compartilhado == 0) {
                uint64_t q1 = N1 / fator_compartilhado;
                uint64_t q2 = N2 / fator_compartilhado;

                std::cout << "  [OK] QUEBRADO em " << passos << " passos usando gap g = " << g << "!\n";
                std::cout << "       Fator Compartilhado (p ou M): " << fator_compartilhado << "\n";
                std::cout << "       Fator q1: " << q1 << " | Fator q2: " << q2 << "\n";

                // Se o fator extraído for par composto (como 46), extrai o primo original por shift
                if (fator_compartilhado % 2 == 0) {
                    std::cout << "       Nota: Fator extraido e composto par. Primo original: "
                              << (fator_compartilhado >> 1) << " (obtido via shift-right)\n";
                }
                quebrado = true;
                break;
            }
        }
    }
    if (!quebrado) {
        std::cout << "  [FALHA] O ataque falhou para o dicionario atual.\n";
    }
}

int main() {
    std::cout << "========================================================================\n";
    std::cout << "             TESTE DE SIMETRIA DE FERMAT E ATAQUE POR GAP              \n";
    std::cout << "========================================================================\n";

    // 1. Validando a tabela de simetria com os seus exemplos reais
    std::cout << "\n1. Mapeamento das Classes de Paridade de Fermat (N % 4):\n";
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "  N   | Factores | N % 4 | Paridade de x | Paridade de y | Coordenadas\n";
    std::cout << "------------------------------------------------------------------------\n";
    FermatState{841, 29, 29}.analisar();
    FermatState{529, 23, 23}.analisar();
    FermatState{667, 23, 29}.analisar();
    std::cout << "------------------------------------------------------------------------\n";

    // 2. Simulando os ataques com primo compartilhado (Exemplos: 23 e 29)
    testarAtaquePorGap(529, 667, "Primos Compartilhados (p = 23, q1 = 23, q2 = 29)");
    testarAtaquePorGap(667, 841, "Primos Compartilhados (p = 29, q1 = 23, q2 = 29)");

    // 3. Simulando o ataque com o multiplicador par composto (M = 46)
    testarAtaquePorGap(506, 598, "Multiplicador Composto Par (M = 46, q1 = 11, q2 = 13)");
    testarAtaquePorGap(598, 782, "Multiplicador Composto Par (M = 46, q1 = 13, q2 = 17)");

    std::cout << "\n========================================================================\n";
    return 0;
}
