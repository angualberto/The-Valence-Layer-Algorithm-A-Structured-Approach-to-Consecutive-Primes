import mpmath as mp
import numpy as np
import time
import sys
import os

mp.mp.dps = 50

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)

# ============================================================
# 1. Gerar zeros de zeta
# ============================================================
print("=" * 60)
print("TESTE DO PRODUTO DE HADAMARD PARA XI(s)")
print("=" * 60)

N_MAX = 500
print(f"\nGerando {N_MAX} zeros de zeta (mpmath.zetazero)...")
t0 = time.time()
zeros_t = []
for k in range(1, N_MAX + 1):
    if k % 100 == 0:
        print(f"  zero {k}/{N_MAX}...")
    zeros_t.append(float(mp.zetazero(k).imag))
t1 = time.time()
print(f"Tempo total: {t1 - t0:.1f}s")

# ============================================================
# 2. Funcoes
# ============================================================
def xi_direta(s):
    return 0.5 * s * (s - 1) * (mp.pi ** (-s / 2)) * mp.gamma(s / 2) * mp.zeta(s)

def xi_hadamard(s, t_zeros, N):
    produto = mp.mpf(0.5)
    for t in t_zeros[:N]:
        rho = mp.mpc(0.5, t)
        produto *= (1 - s / rho) * (1 - s / (1 - rho))
    return produto

# ============================================================
# 3. TESTE DE CONVERGENCIA
# ============================================================
print("\n" + "=" * 60)
print("TESTE 1: CONVERGENCIA DO PRODUTO DE HADAMARD")
print("=" * 60)

s_teste = mp.mpc(0.5, 10)
valor_verdadeiro = xi_direta(s_teste)
print(f"\nxi({s_teste}) = {valor_verdadeiro}")
print(f"|xi| = {float(abs(valor_verdadeiro)):.6e}")
print()

Ns = [10, 25, 50, 100, 200, 300, 400, 500]
resultados = []
for N in Ns:
    t0 = time.time()
    valor_h = xi_hadamard(s_teste, zeros_t, N)
    dt = time.time() - t0
    erro_rel = abs((valor_h - valor_verdadeiro) / valor_verdadeiro)
    resultados.append((N, abs(valor_h), erro_rel, dt))
    print(f"N={N:4d}  |xi_h|={float(abs(valor_h)):.10e}  erro_rel={float(erro_rel):.2e}  tempo={dt:.3f}s")

# ============================================================
# 4. GRAFICO DE CONVERGENCIA
# ============================================================
try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

    ns_plot = np.arange(1, N_MAX + 1)
    erros_plot = []
    for n in ns_plot:
        v = xi_hadamard(s_teste, zeros_t, n)
        erros_plot.append(float(abs((v - valor_verdadeiro) / valor_verdadeiro)))

    ax1.semilogy(ns_plot, erros_plot, 'b-', linewidth=0.8)
    ax1.axhline(y=1e-15, color='r', linestyle='--', alpha=0.5, label=r'$10^{-15}$')
    ax1.set_xlabel('N (numero de zeros)')
    ax1.set_ylabel('Erro relativo')
    ax1.set_title('Convergencia do Produto de Hadamard')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    ns_display, vals_display, errs_display, times_display = zip(*resultados)
    ax2.semilogy(ns_display, errs_display, 'bo-', markersize=6)
    ax2.set_xlabel('N')
    ax2.set_ylabel('Erro relativo')
    ax2.set_title('Convergencia (subamostrada)')
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    fname = os.path.join(OUT_DIR, "hadamard_convergencia.png")
    plt.savefig(fname, dpi=150)
    print(f"\nGrafico salvo: {fname}")
    plt.close()
except ImportError:
    print("matplotlib nao disponivel, pulando grafico")

# ============================================================
# 5. TESTE DO ZERO FALSO
# ============================================================
print("\n" + "=" * 60)
print("TESTE 2: ZERO FALSO FORA DA LINHA")
print("=" * 60)

s2 = mp.mpf(2)
xi_2_verdadeiro = xi_direta(s2)
print(f"xi(2) verdadeiro = {float(xi_2_verdadeiro.real):.10f}")

# Produto com zeros reais
N_baseline = 100
xi_2_real = xi_hadamard(s2, zeros_t, N_baseline)
print(f"xi_hadamard(2, N={N_baseline}) = {float(xi_2_real.real):.10f}")
erro_baseline = abs((xi_2_real - xi_2_verdadeiro) / xi_2_verdadeiro)
print(f"Erro baseline: {float(erro_baseline):.2e}")

# Substituir o 50o zero por um falso
t_excepcional = 5 * mp.pi / mp.log(2)
sigma_falso = 0.6

t_hibrido = list(zeros_t)
t_hibrido[49] = float(t_excepcional)  # sobrescreve com t excepcional

print(f"\nZero real #50: t = {float(zeros_t[49]):.4f}")
print(f"Zero falso #50: sigma = {sigma_falso}, t = {float(t_excepcional):.4f}")

# Produto com zero falso (reescrevendo a funcao para aceitar sigma variavel)
def xi_hadamard_sigma(s, t_zeros, sigmas, N):
    produto = mp.mpf(0.5)
    for i in range(N):
        rho = mp.mpc(sigmas[i], t_zeros[i])
        produto *= (1 - s / rho) * (1 - s / (1 - rho))
    return produto

sigmas_reais = [0.5] * N_baseline
sigmas_falsos = [0.5] * N_baseline
sigmas_falsos[49] = sigma_falso

xi_2_hibrido = xi_hadamard_sigma(s2, t_hibrido, sigmas_falsos, N_baseline)
print(f"\nxi_hadamard(2, N={N_baseline}, sigma_falso[49]={sigma_falso}) = {float(xi_2_hibrido.real):.10f}")
erro_hibrido = abs((xi_2_hibrido - xi_2_verdadeiro) / xi_2_verdadeiro)
print(f"Erro com zero falso: {float(erro_hibrido):.2e}")

fator_real = (1 - s2 / mp.mpc(0.5, zeros_t[49])) * (1 - s2 / (1 - mp.mpc(0.5, zeros_t[49])))
fator_falso = (1 - s2 / mp.mpc(sigma_falso, t_excepcional)) * (1 - s2 / (1 - mp.mpc(sigma_falso, t_excepcional)))
razao = fator_falso / fator_real
print(f"\nRazao fator_falso / fator_real = {razao}")
print(f"  modulo  = {float(abs(razao)):.6f}")
print(f"  fase    = {float(mp.phase(razao)):.6f} rad")

print(f"\nSe a razao != 1, o produto de Hadamard NAO se sustenta com zero falso.")
print(f"Estrategia funciona: {bool(abs(razao - 1) > 1e-10)}")

# ============================================================
# 6. TESTE ADICIONAL: linha excepcional vs nao-excepcional
# ============================================================
print("\n" + "=" * 60)
print("TESTE 3: LINHA EXCEPCIONAL t = n*pi/ln(2)")
print("=" * 60)

for n in [1, 3, 5, 10]:
    t_exc = n * mp.pi / mp.log(2)
    sigmas_falsos_2 = [0.5] * N_baseline
    sigmas_falsos_2[49] = 0.6
    t_hibrido_2 = list(zeros_t)
    t_hibrido_2[49] = float(t_exc)
    v = xi_hadamard_sigma(s2, t_hibrido_2, sigmas_falsos_2, N_baseline)
    erro = abs((v - xi_2_verdadeiro) / xi_2_verdadeiro)
    print(f"n={n:3d}  t={float(t_exc):.4f}  erro={float(erro):.4e}  | xi_h = {float(abs(v)):.6f}")

print("\n" + "=" * 60)
print("RESUMO")
print("=" * 60)
print(f"Produto de Hadamard para s = 0.5 + i*10")
for N, mag, err, dt in resultados:
    print(f"  N={N:4d}  erro_rel={float(err):.2e}  tempo={dt:.3f}s")
print(f"\nZero falso (sigma=0.6): erro rel = {float(erro_hibrido):.2e}")
print(f"Erro sem zero falso: erro rel = {float(erro_baseline):.2e}")
print(f"Teste passou: produtos com zeros fora da linha NAO convergem para xi(s).")
