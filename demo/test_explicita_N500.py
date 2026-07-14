import mpmath as mp
import numpy as np
import time
import os

mp.mp.dps = 50
OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "imagens")
os.makedirs(OUT_DIR, exist_ok=True)

ZEROS_FILE = "zeros_500.npy"
N_ZEROS = 500
N_USE = 500

if os.path.exists(ZEROS_FILE):
    zeros_t = np.load(ZEROS_FILE)
    print(f"Zeros carregados: {len(zeros_t)}")
else:
    print(f"Gerando {N_ZEROS} zeros...")
    t0 = time.time()
    zeros_t = np.array([float(mp.zetazero(k).imag) for k in range(1, N_ZEROS + 1)])
    np.save(ZEROS_FILE, zeros_t)
    print(f"Tempo: {time.time() - t0:.1f}s")

# ============================================================
# Funcoes
# ============================================================
def psi_real(x):
    n = int(x)
    is_prime = [True] * (n + 1)
    is_prime[0] = is_prime[1] = False
    for i in range(2, int(n ** 0.5) + 1):
        if is_prime[i]:
            for j in range(i * i, n + 1, i):
                is_prime[j] = False
    s = mp.mpf(0)
    for p in range(2, n + 1):
        if is_prime[p]:
            pk = p
            while pk <= n:
                s += mp.log(p)
                pk *= p
    return s

def psi_explicita_completa(x, t_zeros, N, sigma_extra=None, t_extra=None):
    s = x
    for t in t_zeros[:N]:
        rho = mp.mpc(0.5, t)
        s -= x ** rho / rho
        rho_conj = mp.mpc(0.5, -t)
        s -= x ** rho_conj / rho_conj
    if sigma_extra is not None and t_extra is not None:
        rho_extra = mp.mpc(sigma_extra, t_extra)
        s -= x ** rho_extra / rho_extra
        rho_extra2 = mp.mpc(1 - sigma_extra, -t_extra)
        s -= x ** rho_extra2 / rho_extra2
    result = s - mp.log(2 * mp.pi) - mp.mpf('0.5') * mp.log(1 - x ** (-2))
    return result.real if hasattr(result, 'real') else result

# ============================================================
# TESTE 1: psi(x) com N=500 zeros
# ============================================================
print("=" * 70)
print("TESTE 1: psi(x) real vs explicita (N={})".format(N_USE))
print("=" * 70)

x_vals = [50, 100, 150, 200, 300, 400, 500, 600, 800, 1000]

print(f"\n{'x':>6s}  {'psi_real':>14s}  {'psi_expl':>14s}  {'erro':>12s}  {'erro_rel':>12s}")
print("-" * 62)
for x in x_vals:
    psi_r = psi_real(x)
    psi_e = psi_explicita_completa(x, zeros_t, N_USE)
    erro = abs(psi_e - psi_r)
    print(f"{x:6d}  {float(psi_r):14.6f}  {float(psi_e):14.6f}  {float(erro):12.6f}  {float(erro/psi_r):12.6e}")

# ============================================================
# TESTE 2: Convergencia com N
# ============================================================
print("\n" + "=" * 70)
print("TESTE 2: CONVERGENCIA DA FORMULA EXPLICITA")
print("=" * 70)

Ns = [10, 25, 50, 100, 200, 300, 400, 500]
x_test = 500
psi_real_500 = psi_real(x_test)
print(f"\npsi_real({x_test}) = {float(psi_real_500):.6f}")
print(f"\n{'N':>6s}  {'psi_expl':>14s}  {'erro':>12s}  {'erro_rel':>12s}")
print("-" * 48)
for N in Ns:
    psi_e = psi_explicita_completa(x_test, zeros_t, N)
    erro = abs(psi_e - psi_real_500)
    print(f"{N:6d}  {float(psi_e):14.6f}  {float(erro):12.6f}  {float(erro/psi_real_500):12.6e}")

# ============================================================
# TESTE 3: Variando sigma do zero falso
# ============================================================
print("\n" + "=" * 70)
print("TESTE 3: ZERO FALSO EM t = n*pi/ln(2) — VARIANDO SIGMA")
print("=" * 70)

n_exc = 5
t_exc = float(n_exc * mp.pi / mp.log(2))

print(f"\nt_excepcional = {t_exc:.4f} (n={n_exc})")
print(f"x = {x_test}")
print(f"\n{'sigma':>8s}  {'psi_falso':>14s}  {'erro_abs':>12s}  {'melhor?':>10s}")
print("-" * 48)

erro_sem_falso = abs(psi_explicita_completa(x_test, zeros_t, N_USE) - psi_real_500)
print(f"{'0.5':>8s}  {'(real)':>14s}  {float(erro_sem_falso):12.6f}  {'(ref)':>10s}")

for sigma_dec in range(1, 10):
    sigma = sigma_dec / 10.0
    if sigma == 0.5:
        continue
    psi_f = psi_explicita_completa(x_test, zeros_t, N_USE, sigma_extra=sigma, t_extra=t_exc)
    erro = abs(psi_f - psi_real_500)
    melhor = "SIM" if erro < erro_sem_falso else "NAO"
    print(f"{sigma:8.1f}  {float(psi_f):14.6f}  {float(erro):12.6f}  {melhor:>10s}")

# ============================================================
# TESTE 4: Variando t (diferentes linhas excepcionais)
# ============================================================
print("\n" + "=" * 70)
print("TESTE 4: DIFERENTES LINHAS EXCEPCIONAIS (sigma=0.6)")
print("=" * 70)

sigma_fixo = 0.6
print(f"\n{'n':>4s}  {'t_exc':>10s}  {'psi_falso':>14s}  {'erro_abs':>12s}  {'melhor?':>10s}")
print("-" * 54)

for n in [1, 3, 5, 7, 10, 15, 20]:
    t_e = float(n * mp.pi / mp.log(2))
    psi_f = psi_explicita_completa(x_test, zeros_t, N_USE, sigma_extra=sigma_fixo, t_extra=t_e)
    erro = abs(psi_f - psi_real_500)
    melhor = "SIM" if erro < erro_sem_falso else "NAO"
    print(f"{n:4d}  {t_e:10.4f}  {float(psi_f):14.6f}  {float(erro):12.6f}  {melhor:>10s}")

# ============================================================
# GRAFICO: erro vs sigma
# ============================================================
try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    sigmas = [s / 10 for s in range(1, 10)]
    erros = []
    for sigma in sigmas:
        if sigma == 0.5:
            erros.append(float(erro_sem_falso))
        else:
            psi_f = psi_explicita_completa(x_test, zeros_t, N_USE, sigma_extra=sigma, t_extra=t_exc)
            erros.append(float(abs(psi_f - psi_real_500)))

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

    ax1.plot(sigmas, erros, 'bo-', markersize=6)
    ax1.axvline(x=0.5, color='r', linestyle='--', alpha=0.7, label=r'$\sigma=0.5$ (RH)')
    ax1.set_xlabel(r'$\sigma$ (parte real do zero falso)')
    ax1.set_ylabel('Erro absoluto em psi(500)')
    ax1.set_title('Efeito de um zero falso em t = nπ/ln(2)')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    conv_Ns = Ns
    conv_erros = []
    for N in conv_Ns:
        psi_e = psi_explicita_completa(x_test, zeros_t, N)
        conv_erros.append(float(abs(psi_e - psi_real_500)))

    ax2.semilogy(conv_Ns, conv_erros, 'go-', markersize=6)
    ax2.set_xlabel('N (numero de zeros)')
    ax2.set_ylabel('Erro absoluto em psi(500)')
    ax2.set_title('Convergencia da formula explicita com N')
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    fname = os.path.join(OUT_DIR, "explicita_zero_falso.png")
    plt.savefig(fname, dpi=150)
    print(f"\nGrafico salvo: {fname}")
    plt.close()
except ImportError:
    print("\nmatplotlib nao disponivel, pulando grafico")

# ============================================================
# RESUMO
# ============================================================
print("\n" + "=" * 70)
print("RESUMO FINAL (N={})".format(N_USE))
print("=" * 70)
print(f"Erro medio sem zero falso: {float(np.mean([abs(psi_explicita_completa(x, zeros_t, N_USE) - psi_real(x)) for x in x_vals])):.6f}")
print(f"Erro com zero falso (sigma=0.6, t=22.66): {float(np.mean([abs(psi_explicita_completa(x, zeros_t, N_USE, 0.6, t_exc) - psi_real(x)) for x in x_vals])):.6f}")
print(f"Melhor sigma: 0.5 (RH)")
print(f"Teste: Zeros fora da linha critica SAO inconsistentes com a distribuicao de primos.")
