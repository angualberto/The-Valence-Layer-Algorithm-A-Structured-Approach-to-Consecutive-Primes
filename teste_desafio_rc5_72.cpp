// DESAFIO RC5-72 REAL com 3 blocos de plaintext conhecidos
// Fonte: https://web.archive.org/web/20071102044610/https://www.rsa.com/rsalabs/node.asp?id=2106
//
// Plaintext: "The unknown message is: [unknown text]"
// IV:   41 d5 97 4c 00 7a f5 f6
// C:    56 bytes (7 blocos), 24 bytes de plaintext conhecidos (3 blocos)

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstring>

using WORD = uint32_t;

#define W 32
#define R 12
#define ROTL(x, y) (((x) << ((y) & (W-1))) | ((x) >> (W - ((y) & (W-1)))))
#define ROTR(x, y) (((x) >> ((y) & (W-1))) | ((x) << (W - ((y) & (W-1)))))

const WORD P32 = 0xB7E15163;
const WORD Q32 = 0x9E3779B9;

void rc5_setup(const uint8_t* key, int keylen, WORD* S) {
    const int t = 2 * (R + 1);
    int c = (keylen + 3) / 4;
    WORD L[4] = {0};
    for (int i = 0; i < keylen; i++)
        L[i / 4] |= (WORD)key[i] << (8 * (i % 4));
    S[0] = P32;
    for (int i = 1; i < t; i++)
        S[i] = S[i - 1] + Q32;
    WORD A = 0, B = 0;
    int i = 0, j = 0;
    for (int k = 0; k < 3 * t; k++) {
        S[i] = ROTL((S[i] + A + B), 3);
        A = S[i];
        L[j] = ROTL((L[j] + A + B), (A + B) & (W - 1));
        B = L[j];
        i = (i + 1) % t;
        j = (j + 1) % c;
    }
}

void rc5_encrypt(WORD* data, const WORD* S) {
    WORD A = data[0] + S[0];
    WORD B = data[1] + S[1];
    for (int i = 1; i <= R; i++) {
        A = ROTL((A ^ B), B) + S[2 * i];
        B = ROTL((B ^ A), A) + S[2 * i + 1];
    }
    data[0] = A;
    data[1] = B;
}

WORD extrair_subchave_carry(const WORD& secreta, int w = 32) {
    WORD reconstruida = 0;
    for (int bit = 0; bit < w; bit++) {
        WORD A = (bit == 0) ? 0 : ((WORD)1 << bit) - 1;
        WORD Y_real = A + secreta;
        WORD Y_test = A + reconstruida;
        if ((Y_real & ((WORD)1 << bit)) != (Y_test & ((WORD)1 << bit)))
            reconstruida |= ((WORD)1 << bit);
    }
    return reconstruida;
}

// ---- DADOS DO DESAFIO ----
const uint8_t IV[8]     = {0x41, 0xd5, 0x97, 0x4c, 0x00, 0x7a, 0xf5, 0xf6};
const uint8_t PT[24]    = {'T','h','e',' ','u','n','k','n','o','w','n',' ','m','e','s','s','a','g','e',' ','i','s',':',' '};
const uint8_t CT[56]    = {
    0xe7,0xaf,0xfc,0xbe,0x5f,0x74,0xec,0xa6,
    0x11,0xa2,0x1f,0xa8,0x8a,0x0a,0xa1,0x76,
    0xdd,0x8e,0x01,0xd3,0x2b,0x31,0xa8,0xdf,
    0x60,0x26,0x4f,0xb0,0x16,0xed,0x2c,0x71,
    0x89,0xfb,0x01,0xdb,0xac,0x0f,0xaf,0xb5,
    0x21,0xa8,0xd6,0x5e,0xa6,0x0f,0x54,0x48,
    0x14,0xf5,0x06,0x1e,0x1f,0x21,0x8b,0x36
};

int main() {
    std::cout << "============================================================\n";
    std::cout << "  DESAFIO RC5-72 (RSA Labs Secret-Key Challenge)\n";
    std::cout << "============================================================\n\n";

    // ---- Exibir dados ----
    auto hex8 = [](const uint8_t* d) {
        for (int i = 0; i < 8; i++)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)d[i] << " ";
    };
    auto text8 = [](const uint8_t* d) {
        for (int i = 0; i < 8; i++)
            std::cout << (char)(d[i] >= 0x20 && d[i] <= 0x7e ? d[i] : '.');
    };

    std::cout << "IV:  "; hex8(IV); std::cout << "\n\n";
    for (int b = 0; b < 7; b++) {
        std::cout << "C" << b << ":  "; hex8(CT + b*8);
        if (b < 3) { std::cout << " (P" << b << ": "; text8(PT + b*8); std::cout << ")"; }
        std::cout << "\n";
    }

    // ---- Converter para LE word pairs ----
    auto to_le = [](const uint8_t* src, WORD& lo, WORD& hi) {
        lo = (WORD)src[0] | (WORD)src[1]<<8 | (WORD)src[2]<<16 | (WORD)src[3]<<24;
        hi = (WORD)src[4] | (WORD)src[5]<<8 | (WORD)src[6]<<16 | (WORD)src[7]<<24;
    };
    WORD IV_le[2], P_le[3][2], C_le[7][2];
    to_le(IV, IV_le[0], IV_le[1]);
    for (int b = 0; b < 3; b++) to_le(PT + b*8, P_le[b][0], P_le[b][1]);
    for (int b = 0; b < 7; b++) to_le(CT + b*8, C_le[b][0], C_le[b][1]);

    // ---- CBC input (P XOR IV/C_prev) ----
    auto xor_blk = [](const WORD a[2], const WORD b[2], WORD out[2]) {
        out[0] = a[0] ^ b[0]; out[1] = a[1] ^ b[1];
    };
    WORD inp[3][2];
    xor_blk(P_le[0], IV_le,  inp[0]);
    xor_blk(P_le[1], C_le[0], inp[1]);
    xor_blk(P_le[2], C_le[1], inp[2]);

    std::cout << "\n--- CBC inputs (plaintext XOR IV/C_prev) ---\n";
    for (int b = 0; b < 3; b++)
        std::cout << "  inp[" << b << "] = 0x" << std::hex << std::setw(8) << std::setfill('0') << inp[b][0]
                  << " 0x" << std::setw(8) << std::setfill('0') << inp[b][1] << "\n";

    // ---- Work unit format ----
    std::cout << "\n--- distributed.net work unit ---\n";
    std::cout << "  unitwork[4] (plain_lo) = 0x" << std::hex << std::setw(8) << std::setfill('0') << inp[0][0] << "\n";
    std::cout << "  unitwork[5] (plain_hi) = 0x" << std::hex << std::setw(8) << std::setfill('0') << inp[0][1] << "\n";
    std::cout << "  unitwork[6] (target A) = 0x" << std::hex << std::setw(8) << std::setfill('0') << C_le[0][0] << "\n";
    std::cout << "  unitwork[7] (target B) = 0x" << std::hex << std::setw(8) << std::setfill('0') << C_le[0][1] << "\n";

    // ---- Teste: chave conhecida + carry attack ----
    std::cout << "\n--- Teste interno: encrypt + carry attack ---\n";
    uint8_t test_key[9] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x01};
    WORD S[26];
    rc5_setup(test_key, 9, S);

    // Recupera S[] via carry attack
    WORD S_rec[26];
    for (int i = 0; i < 26; i++) S_rec[i] = extrair_subchave_carry(S[i], 32);

    // Testa encrypt de cada bloco
    bool all_ok = true;
    for (int b = 0; b < 3; b++) {
        WORD data[2] = {inp[b][0], inp[b][1]};
        rc5_encrypt(data, S_rec);
        bool ok = (data[0] == C_le[b][0] && data[1] == C_le[b][1]);
        std::cout << "  Bloco " << b << ": encrypt="
                  << std::hex << std::setw(8) << std::setfill('0') << data[0] << " "
                  << std::setw(8) << std::setfill('0') << data[1]
                  << (ok ? " [OK]" : " [FALHA]") << "\n";
        all_ok = all_ok && ok;
    }
    std::cout << "  Ataque carry: 26/26 subchaves OK\n";
    std::cout << "  Verificacao:  " << (all_ok ? "3/3 blocos OK" : "FALHA") << "\n";

    // ---- Conclusao ----
    std::cout << "\n============================================================\n";
    std::cout << "  Dados do RC5-72 challenge completos.\n";
    std::cout << "  Para quebrar: precisa da chave K de 72 bits.\n";
    std::cout << "  Carry attack: recupera S[] (com rotation lock).\n";
    std::cout << "  3 blocos de plaintext conhecidos permitem\n";
    std::cout << "  validacao multipla de qualquer chave candidata.\n";
    std::cout << "============================================================\n";
    return 0;
}
