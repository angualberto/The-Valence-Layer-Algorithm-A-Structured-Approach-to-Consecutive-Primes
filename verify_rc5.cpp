#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>

using WORD = uint32_t;

#define W 32
#define R 12
#define ROTL(x, y) (((x) << (y)) | ((x) >> (W - (y))))
#define ROTR(x, y) (((x) >> (y)) | ((x) << (W - (y))))

const WORD P = 0xB7E15163;
const WORD Q = 0x9E3779B9;

void rc5_setup(const uint8_t* key, int keylen, WORD* S) {
    int i, j, k;
    WORD A, B;
    int c = (keylen + 3) / 4;
    WORD L[10];
    memset(L, 0, sizeof(L));

    for (i = 0; i < keylen; i++) {
        L[i / 4] |= (WORD)key[i] << (8 * (i % 4));
    }

    S[0] = P;
    for (i = 1; i < 2 * (R + 1); i++) {
        S[i] = S[i - 1] + Q;
    }

    i = j = 0;
    A = B = 0;
    for (k = 0; k < 3 * (2 * (R + 1)); k++) {
        S[i] = ROTL((S[i] + A + B), 3);
        A = S[i];
        L[j] = ROTL((L[j] + A + B), (A + B) & (W - 1));
        B = L[j];
        i = (i + 1) % (2 * (R + 1));
        j = (j + 1) % c;
    }
}

void rc5_encrypt_block(WORD* data, WORD* S) {
    WORD A = data[0] + S[0];
    WORD B = data[1] + S[1];
    int i;

    for (i = 1; i <= R; i++) {
        A = ROTL((A ^ B), B & (W - 1)) + S[2 * i];
        B = ROTL((B ^ A), A & (W - 1)) + S[2 * i + 1];
    }

    data[0] = A;
    data[1] = B;
}

void rc5_decrypt_block(WORD* data, WORD* S) {
    WORD A = data[0];
    WORD B = data[1];
    int i;

    for (i = R; i >= 1; i--) {
        B = ROTR((B - S[2 * i + 1]), A & (W - 1)) ^ A;
        A = ROTR((A - S[2 * i]), B & (W - 1)) ^ B;
    }

    data[0] = A - S[0];
    data[1] = B - S[1];
}

bool verify_rc5_subkeys(WORD* S) {
    uint8_t iv[8] = {0x41, 0xd5, 0x97, 0x4c, 0x00, 0x7a, 0xf5, 0xf6};
    uint8_t plaintext[8] = {0x54, 0x68, 0x65, 0x20, 0x75, 0x6e, 0x6b, 0x6e};
    uint8_t expected_ciphertext[8] = {0xe7, 0xaf, 0xfc, 0xbe, 0x5f, 0x74, 0xec, 0xa6};

    WORD block[2];
    WORD iv_block[2];

    iv_block[0] = (WORD)iv[0] | ((WORD)iv[1] << 8) | ((WORD)iv[2] << 16) | ((WORD)iv[3] << 24);
    iv_block[1] = (WORD)iv[4] | ((WORD)iv[5] << 8) | ((WORD)iv[6] << 16) | ((WORD)iv[7] << 24);

    block[0] = (WORD)plaintext[0] | ((WORD)plaintext[1] << 8) | ((WORD)plaintext[2] << 16) | ((WORD)plaintext[3] << 24);
    block[1] = (WORD)plaintext[4] | ((WORD)plaintext[5] << 8) | ((WORD)plaintext[6] << 16) | ((WORD)plaintext[7] << 24);

    block[0] ^= iv_block[0];
    block[1] ^= iv_block[1];

    rc5_encrypt_block(block, S);

    uint8_t computed_ciphertext[8];
    computed_ciphertext[0] = block[0] & 0xFF;
    computed_ciphertext[1] = (block[0] >> 8) & 0xFF;
    computed_ciphertext[2] = (block[0] >> 16) & 0xFF;
    computed_ciphertext[3] = (block[0] >> 24) & 0xFF;
    computed_ciphertext[4] = block[1] & 0xFF;
    computed_ciphertext[5] = (block[1] >> 8) & 0xFF;
    computed_ciphertext[6] = (block[1] >> 16) & 0xFF;
    computed_ciphertext[7] = (block[1] >> 24) & 0xFF;

    for (int i = 0; i < 8; i++) {
        if (computed_ciphertext[i] != expected_ciphertext[i]) {
            std::cout << "[FALHA] Byte " << i << " esperado 0x" << std::hex << (int)expected_ciphertext[i] 
                      << " mas obteve 0x" << (int)computed_ciphertext[i] << std::dec << std::endl;
            return false;
        }
    }
    return true;
}

int main() {
    std::cout << "================================================\n";
    std::cout << "  VERIFICADOR RC5-72 (C0 vs IV + P0)\n";
    std::cout << "================================================\n\n";

    uint8_t key_example[9] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x01};
    WORD S_example[26];
    rc5_setup(key_example, 9, S_example);

    std::cout << "[TESTE 1] Chave exemplo (NAO e a chave real):\n";
    for (int i = 0; i < 26; i++) {
        std::cout << "S[" << std::setw(2) << i << "] = 0x" 
                  << std::hex << std::setw(8) << std::setfill('0') << S_example[i] << std::dec << std::endl;
    }

    if (verify_rc5_subkeys(S_example)) {
        std::cout << "\n[OK] C0 coincide!\n";
    } else {
        std::cout << "\n[FALHA] C0 nao coincide.\n";
    }

    std::cout << "\n================================================\n";
    std::cout << " COMO USAR COM O ATAQUE CARRY:\n";
    std::cout << "  1. Execute o ataque carry (teste_72_completo.cpp)\n";
    std::cout << "  2. Ele recupera S[0..25].\n";
    std::cout << "  3. Chame verify_rc5_subkeys(S_recovered).\n";
    std::cout << "  4. Se retornar TRUE, VOCE ENCONTROU A CHAVE DO DESAFIO!\n";
    std::cout << "================================================\n";

    return 0;
}
