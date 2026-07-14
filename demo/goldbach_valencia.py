"""
Goldbach conjecture test using Valence Layer Algorithm framework.
For each even N, counts prime pairs (p, N-p).
"""
import gmpy2
import numpy as np
import matplotlib.pyplot as plt
import time
from math import log

def goldbach_particoes(N, max_examples=3):
    if N < 4 or N % 2 != 0:
        return 0, []
    if N == 4:
        return 1, [(2, 2)]
    count = 0
    examples = []
    p = gmpy2.mpz(3)
    half = N // 2
    while p <= half:
        if gmpy2.is_prime(N - p):
            count += 1
            if len(examples) < max_examples:
                examples.append((int(p), int(N - p)))
        p = gmpy2.next_prime(p + 1)
    return count, examples

def r_assintotico(N):
    C = 0.6601618158468696
    lnN = log(N)
    r = 2.0 * C * N / (lnN * lnN)
    n = N
    p = 3
    while p * p <= n:
        if n % p == 0:
            while n % p == 0:
                n //= p
            r *= (p - 1.0) / (p - 2.0)
        p += 2
    if n > 1 and n != 2:
        r *= (n - 1.0) / (n - 2.0)
    return r

print("="*60)
print("GOLDBACH - ALGORITMO DA CAMADA DE VALENCIA")
print("="*60)

# Teste 1: Pequenos
print("\n--- Teste 1: N pequenos ---")
for N in [10, 50, 100, 200, 500, 1000]:
    r, ex = goldbach_particoes(N)
    print(f"  N={N:4d}: r(N)={r:3d}  assint={r_assintotico(N):.1f}  ex={ex}")

# Teste 2: Medios (potencias de 10)
print("\n--- Teste 2: N medios ---")
for exp in [4, 5, 6, 7]:
    N = 10**exp
    if N % 2 != 0:
        N += 1
    t0 = time.time()
    r, ex = goldbach_particoes(N)
    t1 = time.time()
    ra = r_assintotico(N)
    print(f"  N=10^{exp}: r(N)={r:>6d}  assint={ra:>6.1f}  razao={r/ra:.3f}  tempo={t1-t0:.1f}s")

# Teste 3: Crescimento ate 2000
print("\n--- Teste 3: r(N) para N=10..2000 ---")
Ns = list(range(10, 2001, 2))
rs = []
for N in Ns:
    r, _ = goldbach_particoes(N, 0)
    rs.append(r)
r_ass = [r_assintotico(N) for N in Ns]

fig, axes = plt.subplots(1, 3, figsize=(18, 5))

ax1 = axes[0]
ax1.plot(Ns, rs, 'o-', markersize=2, color='navy')
ax1.set_xlabel('N'); ax1.set_ylabel('r(N)')
ax1.set_title('Particoes de Goldbach vs N'); ax1.grid(alpha=0.3)

ax2 = axes[1]
ax2.plot(Ns, rs, 'o', markersize=2, alpha=0.5, label='obs')
ax2.plot(Ns, r_ass, '-', color='red', lw=2, label='assint')
ax2.set_xlabel('N'); ax2.set_ylabel('r(N)')
ax2.set_title('Obs vs Assintotico'); ax2.legend(); ax2.grid(alpha=0.3)

ax3 = axes[2]
razao = [rs[i]/r_ass[i] if r_ass[i] > 0 else 0 for i in range(len(Ns))]
ax3.plot(Ns, razao, '-', color='green')
ax3.axhline(y=1, color='red', ls='--', label='razao=1')
ax3.set_xlabel('N'); ax3.set_ylabel('obs / assint')
ax3.set_title('Convergencia'); ax3.legend(); ax3.grid(alpha=0.3)

plt.suptitle('Conjectura de Goldbach - Camada de Valencia', fontweight='bold')
plt.tight_layout()
plt.savefig('C:\\Users\\Andre\\Documents\\valence-layer-algorithm\\imagens\\goldbach.png', dpi=300)
print("Grafico: goldbach.png")
