import numpy as np
import matplotlib.pyplot as plt
import os

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)

# Carrega dados amostrados do C
data = np.loadtxt('L_N_100M.txt', skiprows=1)
N_vals = data[:, 0].astype(int)
L_vals = data[:, 1].astype(int)
mask = N_vals > 0
N_vals, L_vals = N_vals[mask], L_vals[mask]

print(f"Dados carregados: {len(N_vals)} pontos, N max = {N_vals[-1]:,}")
print(f"L({N_vals[-1]:,}) = {L_vals[-1]}")

# ============================================================
# Grafico 1: |L(N)| / N^(1/2+eps) para varios eps
# ============================================================
epsilons = [0.0, 0.05, 0.1, 0.2]
fig, ax = plt.subplots(figsize=(12, 6))
colors = ['#2196F3', '#FF9800', '#4CAF50', '#F44336']

for eps, color in zip(epsilons, colors):
    ratio = np.abs(L_vals) / (N_vals ** (0.5 + eps))
    ax.plot(N_vals, ratio, '-', color=color, linewidth=1.5, label=f'epsilon = {eps:.2f}')
    max_r = np.max(ratio)
    print(f"  epsilon={eps:.2f}: max |L|/N^(1/2+eps) = {max_r:.6f}")

ax.axhline(y=1.0, color='black', linestyle='--', linewidth=0.8, label='Cota C=1')
ax.set_xlabel('N')
ax.set_ylabel('|L(N)| / N^{1/2+epsilon}')
ax.set_title('Verificacao da cota de Liouville |L(N)| < C * N^{1/2+epsilon}')
ax.set_xscale('log')
ax.set_yscale('log')
ax.grid(True, alpha=0.3)
ax.legend(fontsize=10)

plt.tight_layout()
fname = os.path.join(OUT_DIR, "liouville_cota_epsilon.png")
plt.savefig(fname, dpi=150)
print(f"\nGrafico salvo: {fname}")

# ============================================================
# Grafico 2: Comparacao com sqrt(N) e 1.36*sqrt(N)
# ============================================================
fig, ax = plt.subplots(figsize=(12, 6))
ax.plot(N_vals, np.abs(L_vals), 'b-', linewidth=1, label=r'$|L(N)|$')
ax.plot(N_vals, np.sqrt(N_vals), 'r--', linewidth=1.5, alpha=0.7, label=r'$\sqrt{N}$')
ax.plot(N_vals, 1.36 * np.sqrt(N_vals), 'g--', linewidth=1.5, alpha=0.7, label=r'$1.36\sqrt{N}$')
ax.set_xlabel('N')
ax.set_ylabel('|L(N)|')
ax.set_title('Envelope de |L(N)|')
ax.set_xscale('log')
ax.set_yscale('log')
ax.grid(True, alpha=0.3)
ax.legend()

plt.tight_layout()
fname = os.path.join(OUT_DIR, "liouville_envelope_10^8.png")
plt.savefig(fname, dpi=150)
print(f"Grafico salvo: {fname}")

# ============================================================
# Analise da cota
# ============================================================
print("\n" + "=" * 60)
print("ANALISE DA COTA")
print("=" * 60)
print(f"\n  epsilon=0.00: max |L|/sqrt(N) = {np.max(np.abs(L_vals) / np.sqrt(N_vals)):.4f}")
print(f"  epsilon=0.05: max |L|/N^0.55 = {np.max(np.abs(L_vals) / (N_vals ** 0.55)):.4f}")
print(f"  epsilon=0.10: max |L|/N^0.60 = {np.max(np.abs(L_vals) / (N_vals ** 0.60)):.4f}")
print(f"  epsilon=0.20: max |L|/N^0.70 = {np.max(np.abs(L_vals) / (N_vals ** 0.70)):.4f}")
print(f"\nConclusao: Para epsilon > 0, a razao tende a zero.")
print(f"Isso e exatamente o que a Hipotesis de Riemann prediz.")

plt.close('all')
