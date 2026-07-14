import numpy as np
import math
import os

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)

# ============================================================
# Carregar dados dos 3 arquivos
# ============================================================
DATA_106 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "L_N_1M.txt")
DATA_107 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "L_N_10M.txt")
DATA_108 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "L_N_100M.txt")

def load_data(filename, label):
    if not os.path.exists(filename):
        print(f"Arquivo nao encontrado: {filename}")
        return None, None, None
    data = np.loadtxt(filename, skiprows=1)
    if data.ndim == 1:
        ns = np.array([int(data[0])])
        ls = np.array([int(data[1])])
    else:
        ns = data[:, 0].astype(int)
        ls = data[:, 1].astype(int)
    print(f"{label}: {len(ns)} pontos, L({ns[-1]:,}) = {ls[-1]}")
    return ns, ls, label

datasets = []
for fpath, label in [(DATA_106, "10^6"), (DATA_107, "10^7"), (DATA_108, "10^8")]:
    result = load_data(fpath, label)
    if result[0] is not None:
        datasets.append(result)

if not datasets:
    print("Nenhum dado carregado!")
    exit(1)

# ============================================================
# Grafico
# ============================================================
try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(2, 2, figsize=(15, 10))
    colors = {'10^6': '#2196F3', '10^7': '#FF9800', '10^8': '#4CAF50'}

    # 1: L(N) para cada escala (subplot separado)
    ax = axes[0, 0]
    for ns, ls, label in datasets:
        scaled_ns = ns / max(ns) * 100
        scaled_ls = ls / math.sqrt(max(ns)) * 10
        ax.plot(ns, ls, '-', linewidth=0.8, color=colors[label], label=label)
    ax.axhline(y=0, color='gray', linestyle='-', linewidth=0.3)
    ax.set_xlabel('N')
    ax.set_ylabel('L(N)')
    ax.set_title('L(N) nas 3 escalas')
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # 2: |L(N)|/sqrt(N)
    ax = axes[0, 1]
    for ns, ls, label in datasets:
        ratio = np.where(ns > 0, np.abs(ls) / np.sqrt(ns), 0)
        ax.plot(ns / 1e6, ratio, '-', linewidth=1.2, color=colors[label], label=label)
    ax.axhline(y=1.0, color='red', linestyle='--', alpha=0.5, label=r'$1.0\sqrt{N}$')
    ax.axhline(y=1.36, color='purple', linestyle=':', alpha=0.7, label=r'max=1.36 ($10^8$)')
    ax.set_xlabel('N (milhoes)')
    ax.set_ylabel(r'$|L(N)|/\sqrt{N}$')
    ax.set_title(r'Razao $|L(N)|/\sqrt{N}$')
    ax.legend(fontsize=8)
    ax.set_ylim(0, 2.0)
    ax.grid(True, alpha=0.3)

    # 3: log-log
    ax = axes[1, 0]
    for ns, ls, label in datasets:
        mask = ns > 0
        ax.loglog(ns[mask], np.abs(ls[mask]), '-', linewidth=1.2, color=colors[label], label=label)
    ns_dense = np.logspace(1, 8, 100)
    ax.loglog(ns_dense, np.sqrt(ns_dense), 'k--', linewidth=1, alpha=0.5, label=r'$\sqrt{N}$')
    ax.loglog(ns_dense, 1.36 * np.sqrt(ns_dense), 'k:', linewidth=1, alpha=0.5, label=r'$1.36\sqrt{N}$')
    ax.set_xlabel('N')
    ax.set_ylabel('|L(N)|')
    ax.set_title('Envelope log-log (3 escalas)')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    # 4: Tabela de resultados
    ax = axes[1, 1]
    ax.axis('off')
    tabela = []
    for ns, ls, label in datasets:
        n_max = ns[-1]
        l_final = ls[-1]
        max_abs = np.max(np.abs(ls))
        max_ratio = np.max(np.where(ns > 0, np.abs(ls) / np.sqrt(ns), 0))
        tabela.append([label, f"{n_max:,}", f"{l_final:,}", f"{max_abs:,}", f"{max_ratio:.4f}"])
    
    col_labels = ['N', 'L(N)', 'max|L|', 'max|L|/√N']
    cell_text = [[row[0]] + row[1:] for row in tabela]
    table = ax.table(cellText=cell_text, colLabels=[''] + col_labels,
                     loc='center', cellLoc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.8)
    
    ax.set_title('Comparacao das 3 escalas', fontsize=12, pad=20)

    plt.tight_layout()
    fname = os.path.join(OUT_DIR, "liouville_comparacao_3escalas.png")
    plt.savefig(fname, dpi=150, bbox_inches='tight')
    print(f"Grafico salvo: {fname}")
    plt.close()
    
except ImportError as e:
    print(f"Erro matplotlib: {e}")

print("\nDone!")
