"""quebra_rsa_gap.py — Ataque por inversão de gap
Uso: python quebra_rsa_gap.py <N1_hex> <N2_hex>
     python quebra_rsa_gap.py --gera <bits>   # gera par RSA e quebra

Ex: python quebra_rsa_gap.py 0x6a55...7551 0x6a55...05bd
"""

import sys, math, random, time

def isprime(n):
    if n < 2: return False
    if n % 2 == 0: return n == 2
    d, s = n-1, 0
    while d % 2 == 0: d //= 2; s += 1
    for a in [2, 3, 5, 7, 11, 13, 17]:
        if a >= n: continue
        x = pow(a, d, n)
        if x == 1 or x == n-1: continue
        for _ in range(s-1):
            x = pow(x, 2, n)
            if x == n-1: break
        else: return False
    return True

def ataque_gap(N1, N2):
    if N2 < N1: N1, N2 = N2, N1
    delta = N2 - N1
    bits = max(N1.bit_length(), N2.bit_length())
    print(f"N1       = {N1:#x}")
    print(f"N2       = {N2:#x}")
    print(f"N1 bits  = {N1.bit_length()}")
    print(f"N2 bits  = {N2.bit_length()}")
    print(f"Delta    = {delta:#x}")
    print()
    t0 = time.time()
    for g in range(2, 502, 2):
        if delta % g != 0: continue
        p = delta // g
        if N1 % p == 0:
            t = time.time() - t0
            q1 = N1 // p
            q2 = N2 // p
            print(f"=== RSA QUEBRADO em {t*1000:.2f}ms ===")
            print(f"gap_q    = {g}")
            print(f"passos   = {g//2}")
            print(f"p        = {p:#x}")
            print(f"q1       = {q1:#x}")
            print(f"q2       = {q2:#x}")
            print(f"N1 % p   = {N1 % p}")
            print(f"N2 % p   = {N2 % p}")
            return p
    print("FALHA: nenhum gap 2..500 funcionou")
    return None

def gera_par(bits):
    print(f"Gerando primos de {bits} bits...")
    p = random.getrandbits(bits) | (1<<(bits-1)) | 1
    while not isprime(p): p += 2
    q1 = random.getrandbits(bits) | (1<<(bits-1)) | 1
    while not isprime(q1) or q1 == p: q1 += 2
    q2 = q1 + 2
    while not isprime(q2): q2 += 2
    N1 = p * q1
    N2 = p * q2
    gap_q = q2 - q1
    print(f"p gerado  = {p:#x}")
    print(f"gap_q     = {gap_q}")
    print()
    ataque_gap(N1, N2)

if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] == "--gera":
        gera_par(int(sys.argv[2]))
    elif len(sys.argv) == 3:
        N1 = int(sys.argv[1], 16) if sys.argv[1].startswith("0x") else int(sys.argv[1])
        N2 = int(sys.argv[2], 16) if sys.argv[2].startswith("0x") else int(sys.argv[2])
        ataque_gap(N1, N2)
    else:
        print(__doc__)
