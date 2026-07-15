import math, time, random

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

for bits in [16, 20, 24, 28]:
    p = random.getrandbits(bits) | (1<<(bits-1)) | 1
    while not isprime(p): p += 2
    q = random.getrandbits(bits) | (1<<(bits-1)) | 1
    while not isprime(q) or q == p: q += 2
    N = p * q

    inicio = time.time()
    cand = 3
    limite = int(math.isqrt(N))
    tent = 0
    while cand <= limite:
        tent += 1
        if N % cand == 0: break
        cand += 2
    tempo = time.time() - inicio

    print(f"RSA-{2*bits:>3d}: primos {bits:>2d}-bit, sqrt(N)~{limite:>10d}, testou {tent:>10d} impares em {tempo:.3f}s")
    print(f"          -> pela caixa: ~{tent//1000} candidatos (menos, mas cada um exige Miller-Rabin em numero de {2*bits} bits)")
    print()
