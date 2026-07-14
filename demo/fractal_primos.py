"""
Visualizacao fractal dos numeros primos: paridade, gaps, cauda decimal.
"""
import numpy as np
import matplotlib.pyplot as plt
from collections import Counter
from matplotlib.colors import LogNorm
import sympy as sp
import time

# ============================================================
# 1. GERAR PRIMOS EM TRES FAIXAS
# ============================================================
print("Gerando primos...")
t0 = time.time()
primos_pequenos = list(sp.primerange(2, 2000))
primos_medios = []
p = sp.nextprime(10**6)
for _ in range(500):
    primos_medios.append(p); p = sp.nextprime(p+1)
primos_grandes = []
p = sp.nextprime(10**9)
for _ in range(500):
    primos_grandes.append(p); p = sp.nextprime(p+1)
t1 = time.time()
print(f"  Primos pequenos: {len(primos_pequenos)}, medios: {len(primos_medios)}, grandes: {len(primos_grandes)} ({t1-t0:.1f}s)")

# ============================================================
# 2. MAPA DE CALOR: ULTIMOS 2 DIGITOS
# ============================================================
def matriz_ultimos_2(primos):
    c = Counter(p % 100 for p in primos)
    m = np.zeros((10, 10))
    for i in range(10):
        for j in range(10):
            m[i, j] = c.get(i*10 + j, 0)
    return m

m_peq = matriz_ultimos_2(primos_pequenos)
m_med = matriz_ultimos_2(primos_medios)
m_grd = matriz_ultimos_2(primos_grandes)

# ============================================================
# 3. GAPS
# ============================================================
def gaps(p): return [p[i+1]-p[i] for i in range(len(p)-1)]

g_peq = gaps(primos_pequenos)
g_med = gaps(primos_medios)
g_grd = gaps(primos_grandes)

# ============================================================
# 4. PLOT
# ============================================================
fig = plt.figure(figsize=(18, 12))

# Painel 1: Mapa de calor (primos grandes)
ax1 = fig.add_subplot(2, 3, 1)
im = ax1.imshow(m_grd, cmap='YlOrRd', norm=LogNorm(vmin=1, vmax=max(1, m_grd.max())), origin='lower')
ax1.set_title(f'Ultimos 2 digitos\n{len(primos_grandes)} primos ≈ 10⁹')
ax1.set_xlabel('Unidades'); ax1.set_ylabel('Dezenas')
ax1.set_xticks(np.arange(10)); ax1.set_yticks(np.arange(10))
plt.colorbar(im, ax=ax1)

# Painel 2: Histograma gaps (auto-similaridade)
ax2 = fig.add_subplot(2, 3, 2)
bins = np.linspace(0, 100, 40)
ax2.hist(g_peq, bins=bins, alpha=0.5, label=f'≤2000 ({len(g_peq)} gaps)', density=True, color='blue')
ax2.hist(g_med, bins=bins, alpha=0.5, label=f'≈10⁶ ({len(g_med)} gaps)', density=True, color='green')
ax2.hist(g_grd, bins=bins, alpha=0.5, label=f'≈10⁹ ({len(g_grd)} gaps)', density=True, color='red')
ax2.set_xlabel('Gap'); ax2.set_ylabel('Densidade')
ax2.set_title('Distribuicao dos gaps (auto-similaridade)')
ax2.legend(fontsize=8)

# Painel 3: Gaps vs indice (flutuacoes)
ax3 = fig.add_subplot(2, 3, 3)
ax3.plot(g_peq, '.-', markersize=2, alpha=0.6, label=f'≤2000', linewidth=0.5)
ax3.plot(range(len(g_med)), g_med, '.-', markersize=2, alpha=0.6, label=f'≈10⁶', linewidth=0.5)
ax3.plot(range(len(g_grd)), g_grd, '.-', markersize=2, alpha=0.6, label=f'≈10⁹', linewidth=0.5)
ax3.set_xlabel('Indice do gap'); ax3.set_ylabel('Gap')
ax3.set_title('Estrutura local dos gaps')
ax3.legend(fontsize=8)

# Painel 4: Arvore da paridade (fractal)
ax4 = fig.add_subplot(2, 3, 4)
prof = 7
ax4.set_xlim(-0.5, 0.5); ax4.set_ylim(0, prof+0.5)
ax4.axis('off')
for nivel in range(prof):
    y = nivel + 0.5
    n = 2 ** nivel
    esp = 1.0 / (n + 1)
    for i in range(n):
        x = (i + 1) * esp - 0.5
        size = 300 / (nivel + 1)
        ax4.scatter(x, y, s=size, c='darkblue', zorder=3, alpha=0.8)
        if nivel > 0:
            pai = i // 2
            px = (pai + 1) * (1.0 / (2**(nivel-1) + 1)) - 0.5
            py = nivel - 0.5
            ax4.plot([px, x], [py, y], 'gray', linewidth=0.4, alpha=0.5)
ax4.text(0, 0.1, '1 (impar)', ha='center', fontsize=10, fontweight='bold')
ax4.text(0, 1.1, '2 (par)', ha='center', fontsize=10)
ax4.set_title('Arvore da paridade\n(0.5 × 2 = 1)', fontsize=11)

# Painel 5: Mapa para primos pequenos
ax5 = fig.add_subplot(2, 3, 5)
im5 = ax5.imshow(m_peq, cmap='Blues', norm=LogNorm(vmin=1, vmax=max(1, m_peq.max())), origin='lower')
ax5.set_title(f'Ultimos 2 digitos\n{len(primos_pequenos)} primos ≤ 2000')
ax5.set_xlabel('Unidades'); ax5.set_ylabel('Dezenas')
ax5.set_xticks(np.arange(10)); ax5.set_yticks(np.arange(10))
plt.colorbar(im5, ax=ax5)

# Painel 6: Mapa para primos medios
ax6 = fig.add_subplot(2, 3, 6)
im6 = ax6.imshow(m_med, cmap='Greens', norm=LogNorm(vmin=1, vmax=max(1, m_med.max())), origin='lower')
ax6.set_title(f'Ultimos 2 digitos\n{len(primos_medios)} primos ≈ 10⁶')
ax6.set_xlabel('Unidades'); ax6.set_ylabel('Dezenas')
ax6.set_xticks(np.arange(10)); ax6.set_yticks(np.arange(10))
plt.colorbar(im6, ax=ax6)

plt.suptitle('Estrutura Fractal dos Numeros Primos — Algoritmo da Camada de Valencia',
             fontsize=14, fontweight='bold')
plt.tight_layout()
plt.savefig('C:\\Users\\Andre\\Documents\\valence-layer-algorithm\\fractal_primos.png',
            dpi=300, bbox_inches='tight')
print(f"Grafico salvo: fractal_primos.png")
print("\nEstatisticas:")
print("  Pequenos (<=2000): %d primos, gap medio=%.1f" % (len(primos_pequenos), np.mean(g_peq)))
print("  Medios (~10^6):    %d primos, gap medio=%.1f" % (len(primos_medios), np.mean(g_med)))
print("  Grandes (~10^9):   %d primos, gap medio=%.1f" % (len(primos_grandes), np.mean(g_grd)))
print("  ln(2000)=%.1f, ln(10^6)=%.1f, ln(10^9)=%.1f" % (np.log(2000), np.log(1e6), np.log(1e9)))
