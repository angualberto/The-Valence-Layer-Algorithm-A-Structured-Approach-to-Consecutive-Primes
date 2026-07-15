// ataque_rsa256.c — Inversão por gap para RSA-256 (primos 256-bit)
// Uso: ataque_rsa256 <N1_hex> <N2_hex>
#include "bigint_min.h"
#include <stdio.h>
#include <string.h>

// Converte hex string para array de bytes
static int hex_to_bytes(const char* hex, unsigned char* out, int max_len) {
    int len = strlen(hex);
    int byte_len = len / 2;
    if (byte_len > max_len) byte_len = max_len;
    for (int i = 0; i < byte_len; i++) {
        unsigned int b;
        sscanf(hex + 2 * i, "%2x", &b);
        out[i] = (unsigned char)b;
    }
    return byte_len;
}

static int atk_gap_bi(const unsigned char* n1b, int n1l,
                      const unsigned char* n2b, int n2l,
                      BI* p_out, w64* passos) {
    BI N1, N2, delta, p, rem;
    bi_from_be(n1b, n1l, &N1);
    bi_from_be(n2b, n2l, &N2);
    *passos = 0;
    if (bi_cmp(&N2, &N1) < 0) return 0;
    bi_sub(&delta, &N2, &N1);

    for (w64 g = 2; g <= 500; g += 2) {
        (*passos)++;
        if (bi_mod_u64(&delta, g) != 0) continue;
        bi_div_u64(&p, &delta, g);
        bi_mod(&rem, &N1, &p);
        if (bi_is_zero(&rem)) { bi_copy(p_out, &p); return 1; }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Uso: ataque_rsa256 <N1_hex> <N2_hex>\n");
        printf("Ex: ataque_rsa256 0x5ea0...540f 0x5ea0...e5bd\n");
        return 1;
    }

    const char* hex1 = argv[1];
    const char* hex2 = argv[2];
    if (hex1[0] == '0' && hex1[1] == 'x') hex1 += 2;
    if (hex2[0] == '0' && hex2[1] == 'x') hex2 += 2;

    unsigned char n1b[128] = {0}, n2b[128] = {0};
    int l1 = hex_to_bytes(hex1, n1b, 128);
    int l2 = hex_to_bytes(hex2, n2b, 128);

    printf("N1 bytes: %d\n", l1);
    printf("N2 bytes: %d\n", l2);

    BI p_bi;
    w64 passos;
    int ok = atk_gap_bi(n1b, l1, n2b, l2, &p_bi, &passos);

    if (ok) {
        printf("SUCESSO! p encontrado em %llu passos.\n", passos);
        printf("p = "); bi_hex(&p_bi); printf("\n");
    } else {
        printf("FALHA: p nao encontrado apos %llu passos.\n", passos);
    }
    return ok ? 0 : 1;
}
