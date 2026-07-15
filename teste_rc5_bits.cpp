#include <iostream>
#include <iomanip>
#include <cstdint>

// Configurações do RC5-32/12/16
const int w = 32;          // Tamanho da palavra em bits
const int r = 12;          // Número de rodadas
const int b = 16;          // Tamanho da chave em bytes
const int t = 26;          // Tamanho da tabela de subchaves: 2 * (r + 1)

using WORD = uint32_t;

// Constantes Mágicas do RC5 (Derivadas de e e Phi)
const WORD P32 = 0xB7E15163;
const WORD Q32 = 0x9E3779B9;

WORD S[t]; // Tabela de subchaves S

// Operação física de rotação à esquerda (odômetro lógico)
inline WORD ROTL(WORD x, WORD y) {
    return ((x << (y & (w - 1))) | (x >> (w - (y & (w - 1)))));
}

// Operação física de rotação à direita
inline WORD ROTR(WORD x, WORD y) {
    return ((x >> (y & (w - 1))) | (x << (w - (y & (w - 1)))));
}

// Setup de expansão de chave por rotação mútua de tabelas
void RC5_SETUP(const uint8_t* K) {
    const int u = w / 8;
    const int c = (b > 0) ? ((b + u - 1) / u) : 1;
    WORD L[4] = {0}; // Para b=16, c=4

    // 1. Converter vetor de bytes K para vetor de palavras L
    for (int i = b - 1; i >= 0; i--) {
        L[i / u] = (L[i / u] << 8) + K[i];
    }

    // 2. Inicializar tabela S usando as constantes irracionais P32 e Q32
    S[0] = P32;
    for (int i = 1; i < t; i++) {
        S[i] = S[i - 1] + Q32;
    }

    // 3. Misturar chave secreta em S e L (Loop de acoplamento mecânico)
    WORD A = 0, B = 0;
    int i = 0, j = 0;
    int total_passos = 3 * ((t > c) ? t : c); // 3 * max(t, c) = 78 passos

    for (int k = 0; k < total_passos; k++) {
        A = S[i] = ROTL(S[i] + (A + B), 3);
        B = L[j] = ROTL(L[j] + (A + B), (A + B));
        i = (i + 1) % t;
        j = (j + 1) % c;
    }
}

// Cifragem direta
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

// Decifragem reversa
void RC5_DECRYPT(const WORD* ct, WORD* pt) {
    WORD B = ct[1];
    WORD A = ct[0];
    for (int i = r; i > 0; i--) {
        B = ROTR(B - S[2 * i + 1], A) ^ A;
        A = ROTR(A - S[2 * i], B) ^ B;
    }
    pt[1] = B - S[1];
    pt[0] = A - S[0];
}

int main() {
    // Chave de teste de 16 bytes (128 bits)
    const uint8_t chave[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    // Bloco de Texto Claro (Plaintext): 2 palavras de 32 bits
    const WORD texto_claro[2] = { 0xFEEDFACE, 0xDEADBEEF };
    WORD texto_cifrado[2] = {0};
    WORD texto_decifrado[2] = {0};

    std::cout << "====================================================\n";
    std::cout << "          PROVA DE CONCEITO DO ALGORITMO RC5        \n";
    std::cout << "====================================================\n";

    // 1. Executar a expansão de chaves (Setup)
    RC5_SETUP(chave);
    std::cout << " [+] Expansao de chave gerada com sucesso.\n";
    std::cout << "     -> S[0] (Constante de entrada P32 misturada): 0x"
              << std::hex << std::setw(8) << std::setfill('0') << S[0] << "\n";

    // 2. Executar a Cifragem
    RC5_ENCRYPT(texto_claro, texto_cifrado);
    std::cout << "----------------------------------------------------\n";
    std::cout << " [IN]  Plaintext  A: 0x" << std::hex << texto_claro[0]
              << " | B: 0x" << texto_claro[1] << "\n";
    std::cout << " [OUT] Ciphertext A: 0x" << std::hex << texto_cifrado[0]
              << " | B: 0x" << texto_cifrado[1] << "\n";

    // 3. Executar a Decifragem (Prova de Simetria Reversa)
    RC5_DECRYPT(texto_cifrado, texto_decifrado);
    std::cout << " [OUT] Decrypted  A: 0x" << std::hex << texto_decifrado[0]
              << " | B: 0x" << texto_decifrado[1] << "\n";
    std::cout << "----------------------------------------------------\n";

    // 4. Validação Lógica dos Bits
    if (texto_claro[0] == texto_decifrado[0] && texto_claro[1] == texto_decifrado[1]) {
        std::cout << " [OK] PROVA BEM SUCEDIDA: Integridade de bits 100% preservada!\n";
    } else {
        std::cout << " [FALHA] FALHA NA PROVA: Os dados divergiram durante a reversao.\n";
    }
    std::cout << "====================================================\n";

    return 0;
}
