import mpmath as mp
import numpy as np
import time

mp.mp.dps = 50

print("=" * 70)
print("FORMULA EXPLICITA: TESTE COM ZERO FALSO")
print("=" * 70)

# ============================================================
# 1. Gerar zeros (salvos do teste anterior, ou regenerar)
# ============================================================
# Tenta carregar do teste_hadamard se existir, senao gera
import sys
import os

ZEROS_FILE = "zeros_500.npy"
if os.path.exists(ZEROS_FILE):
    zeros_t = np.load(ZEROS_FILE)
    N_DISP = len(zeros_t)
    print(f"Zeros carregados: {N_DISP}")
else:
    N_DISP = 200
    print(f"Gerando {N_DISP} zeros...")
    t0 = time.time()
    zeros_t = np.array([float(mp.zetazero(k).imag) for k in range(1, N_DISP + 1)])
    np.save(ZEROS_FILE, zeros_t)
    print(f"Tempo: {time.time() - t0:.1f}s")

# ============================================================
# 2. Calcular psi(x) real a partir de potencias de primos
# ============================================================
def psi_real(x):
    """Computa psi(x) = sum_{p^k <= x} log(p) via crivo"""
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

# ============================================================
# 3. Formula explicita com N zeros
# ============================================================
def psi_explicita(x, t_zeros, N, sigma=0.5):
    """psi(x) = x - sum_{rho} x^rho/rho - log(2*pi) - 0.5*log(1 - x^-2)"""
    s = x
    for t in t_zeros[:N]:
        rho = mp.mpc(sigma, t)
        s -= x ** rho / rho
        # parceiro 1-rho (ja incluso no pareamento se sigma=0.5)
        if sigma != 0.5:
            rho2 = mp.mpc(1 - sigma, t)
            s -= x ** rho2 / rho2
    # Se sigma=0.5, o termo (1-rho) = conjugado, e o somatorio ja cobre ambos
    # porque iteramos sobre t>0 apenas. Mas precisamos incluir o conjugado explicitamente.
    # Na verdade, a formula padrao soma sobre todos os zeros, e para sigma=0.5
    # os zeros sao (0.5 + it, 0.5 - it). Somando t>0 e t<0 da o dobro da parte real.
    # Vamos refazer: somar sobre t>0 e incluir o conjugado.
    result = s - mp.log(2 * mp.pi) - mp.mpf('0.5') * mp.log(1 - x ** (-2))
    return result.real if hasattr(result, 'real') else result

def psi_explicita_completa(x, t_zeros, N, sigma_extra=None, t_extra=None):
    """psi(x) com zeros reais + um zero extra hipotetico"""
    s = x
    # zeros reais (sigma=0.5, pareados com conjugados)
    for t in t_zeros[:N]:
        rho = mp.mpc(0.5, t)
        s -= x ** rho / rho
        rho_conj = mp.mpc(0.5, -t)
        s -= x ** rho_conj / rho_conj
    # zero extra
    if sigma_extra is not None and t_extra is not None:
        rho_extra = mp.mpc(sigma_extra, t_extra)
        s -= x ** rho_extra / rho_extra
        rho_extra2 = mp.mpc(1 - sigma_extra, -t_extra)
        s -= x ** rho_extra2 / rho_extra2
    result = s - mp.log(2 * mp.pi) - mp.mpf('0.5') * mp.log(1 - x ** (-2))
    return result.real if hasattr(result, 'real') else result

# ============================================================
# 4. TESTE: Comparar psi(x) em varios pontos
# ============================================================
print("\n" + "=" * 70)
print("TESTE: psi(x) real vs explicita (N=200 zeros)")
print("=" * 70)

x_vals = [50, 100, 150, 200, 300, 400, 500, 600, 800, 1000]
N_use = min(200, len(zeros_t))

print(f"\n{'x':>6s}  {'psi_real':>14s}  {'psi_expl':>14s}  {'erro':>12s}  {'erro_rel':>12s}")
print("-" * 62)
for x in x_vals:
    psi_r = psi_real(x)
    psi_e = psi_explicita_completa(x, zeros_t, N_use)
    erro = abs(psi_e - psi_r)
    erro_rel = float(erro / psi_r)
    print(f"{x:6d}  {float(psi_r):14.6f}  {float(psi_e):14.6f}  {float(erro):12.6f}  {erro_rel:12.6e}")

# ============================================================
# 5. TESTE COM ZERO FALSO
# ============================================================
print("\n" + "=" * 70)
print("TESTE: ADICIONANDO UM ZERO FALSO (sigma=0.6, t = n*pi/ln2)")
print("=" * 70)

n_exc = 5
t_exc = n_exc * mp.pi / mp.log(2)
sigma_falso = 0.6

print(f"\nZero falso: sigma = {sigma_falso}, t = {float(t_exc):.4f} (n={n_exc})")

print(f"\n{'x':>6s}  {'psi_real':>14s}  {'psi_real':>14s}  {'psi_falso':>14s}  {'erro_real':>12s}  {'erro_falso':>12s}")
print("-" * 76)
for x in x_vals:
    psi_r = psi_real(x)
    psi_e = psi_explicita_completa(x, zeros_t, N_use)
    psi_f = psi_explicita_completa(x, zeros_t, N_use, sigma_extra=sigma_falso, t_extra=float(t_exc))
    erro_real = abs(psi_e - psi_r)
    erro_falso = abs(psi_f - psi_r)
    melhor = "REAL" if erro_real < erro_falso else "FALSO"
    print(f"{x:6d}  {float(psi_r):14.6f}  {float(psi_e):14.6f}  {float(psi_f):14.6f}  {float(erro_real):12.6f}  {float(erro_falso):12.6f}  {melhor}")

# ============================================================
# 6. TESTE: variando sigma do zero falso
# ============================================================
print("\n" + "=" * 70)
print("TESTE: VARIANDO SIGMA DO ZERO FALSO (x=500)")
print("=" * 70)

x_test = 500
psi_real_500 = psi_real(x_test)
print(f"\npsi_real({x_test}) = {float(psi_real_500):.6f}")

print(f"\n{'sigma':>8s}  {'psi_falso':>14s}  {'erro_abs':>12s}  {'melhora?':>10s}")
print("-" * 46)
erro_sem_falso = abs(psi_explicita_completa(x_test, zeros_t, N_use) - psi_real_500)
print(f"{'--':>8s}  {'--':>14s}  {float(erro_sem_falso):12.6f}  {'(ref)':>10s}")

for sigma in [mp.mpf(s) / 10 for s in range(1, 10)]:
    if sigma == 0.5:
        continue
    psi_f = psi_explicita_completa(x_test, zeros_t, N_use, sigma_extra=float(sigma), t_extra=float(t_exc))
    erro = abs(psi_f - psi_real_500)
    melhora = "SIM" if erro < erro_sem_falso else "NAO"
    print(f"{float(sigma):8.1f}  {float(psi_f):14.6f}  {float(erro):12.6f}  {melhora:>10s}")

# ============================================================
# 7. RESUMO
# ============================================================
print("\n" + "=" * 70)
print("RESUMO")
print("=" * 70)
print(f"Zeros usados: {N_use}")
print(f"Zero falso: sigma={sigma_falso}, t={float(t_exc):.4f} (linha excepcional n={n_exc})")
print(f"Erro medio sem zero falso: {float(np.mean([abs(psi_explicita_completa(x, zeros_t, N_use) - psi_real(x)) for x in x_vals])):.6f}")
print(f"Erro medio com zero falso: {float(np.mean([abs(psi_explicita_completa(x, zeros_t, N_use, sigma_falso, float(t_exc)) - psi_real(x)) for x in x_vals])):.6f}")
