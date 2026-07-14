import numpy as np
import math
import time
import os

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)

SAVE_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "L_N_10M.txt")

N_MAX = 10_000_000
SAVE_INTERVAL = 100_000

print("=" * 70)
print(f"LIQUVILLE L(N) ATE N = {N_MAX:,}")
print("=" * 70)

# ============================================================
# Crivo para omega(n)
# ============================================================
print(f"\nCrivo: calculando omega(n) para n=1..{N_MAX:,}...")
t0 = time.time()

omega = np.zeros(N_MAX + 1, dtype=np.int32)
for p in range(2, N_MAX + 1):
    if omega[p] == 0:
        step = p
        start = p
        for multiple in range(start, N_MAX + 1, step):
            n = multiple
            while n % p == 0:
                omega[multiple] += 1
                n //= p

t1 = time.time()
print(f"Crivo completo em {t1 - t0:.1f}s")

# ============================================================
# lambda(n) e L(N)
# ============================================================
print(f"\nCalculando lambda(n) e L(N)...")
lambda_n = np.where(omega % 2 == 0, np.int8(1), np.int8(-1))
lambda_n[1] = 1
lambda_n[0] = 0

L = np.cumsum(lambda_n)
t2 = time.time()
print(f"Acumulacao em {t2 - t1:.1f}s")

# ============================================================
# Salvar amostras
# ============================================================
print(f"\nSalvando amostras a cada {SAVE_INTERVAL:,}...")
sample_Ns = list(range(0, N_MAX + 1, SAVE_INTERVAL))
if sample_Ns[-1] != N_MAX:
    sample_Ns.append(N_MAX)

sample_Ls = [int(L[n]) if n <= N_MAX else 0 for n in sample_Ns]

with open(SAVE_FILE, 'w') as f:
    f.write("# N  L(N)  |L(N)|/sqrt(N)\n")
    for n, ln in zip(sample_Ns, sample_Ls):
        ratio = abs(ln) / math.sqrt(n) if n > 0 else 0
        f.write(f"{n} {ln} {ratio:.6f}\n")
print(f"Salvo: {SAVE_FILE}")

# ============================================================
# Estatisticas
# ============================================================
L_final = int(L[N_MAX])
L_max = int(np.max(np.abs(L)))
L_max_pos = int(np.argmax(np.abs(L)))
sqrt_N = math.sqrt(N_MAX)
ratio_max = float(np.max(np.abs(L[1:]) / np.sqrt(np.arange(1, N_MAX + 1))))
ratio_max_pos = int(np.argmax(np.abs(L[1:]) / np.sqrt(np.arange(1, N_MAX + 1))))

print(f"\n{'=' * 70}")
print("RESULTADOS")
print(f"{'=' * 70}")
print(f"L({N_MAX:,}) = {L_final}")
print(f"max |L(N)| = {L_max:,}  (em N = {L_max_pos:,})")
print(f"max |L|/sqrt(N) = {ratio_max:.4f}  (em N = {ratio_max_pos:,})")
print(f"sqrt(N) em N_max = {sqrt_N:.2f}")
print(f"Razao final: |L|/sqrt(N) = {abs(L_final) / sqrt_N:.4f}")

# Violacoes de cota
for C in [0.5, 1.0, 1.5, 2.0, 3.0]:
    viol = int(np.sum(np.abs(L) > C * np.sqrt(np.arange(N_MAX + 1))))
    print(f"  Violacoes |L| > {C:.1f}*sqrt(N): {viol:,}")

# ============================================================
# Distribuicao
# ============================================================
count_m1 = int(np.sum(lambda_n == -1))
count_p1 = int(np.sum(lambda_n == 1))
print(f"\nDistribuicao de lambda(n):")
print(f"  +1 (par):   {count_p1:,} ({count_p1/N_MAX*100:.2f}%)")
print(f"  -1 (impar): {count_m1:,} ({count_m1/N_MAX*100:.2f}%)")

# ============================================================
# Grafico
# ============================================================
try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    # 1: L(N) vs N
    ax = axes[0, 0]
    step_plot = max(1, N_MAX // 10000)
    ns = np.arange(0, N_MAX + 1, step_plot)
    ax.plot(ns, L[ns], 'b-', linewidth=0.5)
    ax.plot(ns, np.sqrt(ns), 'r--', linewidth=0.8, alpha=0.5, label=r'$+\sqrt{N}$')
    ax.plot(ns, -np.sqrt(ns), 'r--', linewidth=0.8, alpha=0.5, label=r'$-\sqrt{N}$')
    ax.set_xlabel('N')
    ax.set_ylabel('L(N)')
    ax.set_title(f'Somatória de Liouville L(N) ate N={N_MAX:,}')
    ax.legend()
    ax.grid(True, alpha=0.3)

    # 2: |L(N)|/sqrt(N)
    ax = axes[0, 1]
    ratio_ns = np.where(ns > 0, np.abs(L[ns]) / np.sqrt(ns), 0)
    ax.plot(ns, ratio_ns, 'g-', linewidth=0.8)
    ax.axhline(y=1, color='r', linestyle='--', alpha=0.5, label='1.0')
    ax.axhline(y=ratio_max, color='orange', linestyle=':', alpha=0.7, label=f'max={ratio_max:.3f}')
    ax.set_xlabel('N')
    ax.set_ylabel(r'$|L(N)|/\sqrt{N}$')
    ax.set_title(r'Razao $|L(N)|/\sqrt{N}$')
    ax.grid(True, alpha=0.3)
    ax.legend()

    # 3: log-log
    ax = axes[1, 0]
    ax.loglog(ns[1:], np.abs(L[ns[1:]]), 'b-', linewidth=1, label=r'$|L(N)|$')
    ax.loglog(ns[1:], np.sqrt(ns[1:]), 'r--', linewidth=1, alpha=0.7, label=r'$\sqrt{N}$')
    ax.set_xlabel('N')
    ax.set_ylabel('|L(N)|')
    ax.set_title('Envelope log-log')
    ax.grid(True, alpha=0.3)
    ax.legend()

    # 4: histograma por omega
    ax = axes[1, 1]
    max_k = 8
    counts = [int(np.sum(omega == k)) for k in range(1, max_k + 1)]
    bars = ax.bar(range(1, max_k + 1), counts, color='steelblue')
    for k, c in enumerate(counts, 1):
        if c > 0:
            ax.text(k, c + max(counts) * 0.01, f'{c/N_MAX*100:.1f}%', ha='center', fontsize=8)
    ax.set_xlabel('k = omega(n)')
    ax.set_ylabel('Contagem')
    ax.set_title('Distribuicao do numero de fatores primos')

    plt.tight_layout()
    fname = os.path.join(OUT_DIR, f"liouville_LN_10M.png")
    plt.savefig(fname, dpi=150)
    print(f"\nGrafico salvo: {fname}")
    plt.close()
except ImportError:
    print("\nmatplotlib nao disponivel")

print(f"\nFinal: L({N_MAX:,}) = {L_final}")
print(f"max |L|/sqrt(N) = {ratio_max:.4f}")
print(f"Dados salvos em: {SAVE_FILE}")
