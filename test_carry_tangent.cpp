#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <cstdint>

using WORD = uint32_t;

#define W 32
#define R 12
#define ROTL(x, y) (((x) << (y)) | ((x) >> (W - (y))))

double get_rotation_tangent(WORD A, WORD B) {
    int rot = B & (W - 1);
    double phi = 2.0 * M_PI * rot / W;
    double sin_val = sin(phi);
    double cos_val = cos(phi);
    if (cos_val == 0) return INFINITY;
    return sin_val / cos_val;
}

bool carry_attack_with_tangent(WORD S_secret, int w, WORD &S_recovered, int &locks_found) {
    S_recovered = 0;
    locks_found = 0;

    for (int bit = 0; bit < w; bit++) {
        WORD A = (1ULL << bit) - 1;
        WORD B = 0;

        double tan_val = get_rotation_tangent(A, B);

        if (tan_val == 0.0) {
            locks_found++;
            WORD Y_real = A + S_secret;
            WORD Y_test = A + S_recovered;

            if ((Y_real & (1ULL << bit)) != (Y_test & (1ULL << bit))) {
                S_recovered |= (1ULL << bit);
            }
        } else {
            continue;
        }
    }

    return (S_recovered == S_secret);
}

int main() {
    const int w = 32;
    WORD S_secret = 0x5A3C96E7;
    WORD S_recovered;
    int locks_found;

    std::cout << "================================================\n";
    std::cout << "  ATAQUE CARRY COM DETECCAO DE TANGENTE\n";
    std::cout << "================================================\n";
    std::cout << "Subchave secreta: 0x" << std::hex << S_secret << std::dec << std::endl;

    bool success = carry_attack_with_tangent(S_secret, w, S_recovered, locks_found);

    std::cout << "Locks detectados: " << locks_found << " / " << w << " bits\n";
    std::cout << "Subchave recuperada: 0x" << std::hex << S_recovered << std::dec << std::endl;

    if (success) {
        std::cout << "[OK] Ataque bem-sucedido!\n";
    } else {
        std::cout << "[FALHA] Ataque falhou.\n";
    }

    return 0;
}
