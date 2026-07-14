import mpmath as mp
import numpy as np
import time

mp.mp.dps = 50

print("=" * 70)
print("SIMETRIA K_q(s)K_q(1-s) PARA DIFERENTES BASES q")
print("=" * 70)

def imag_simetria(s, q):
    prod = (q**s - 1) * (q**(1 - s) - 1)
    return mp.im(prod)

# ============================================================
# TESTE 1: Minimo da parte imaginaria no retangulo [0,1]x[0,5]
# ============================================================
print("\n" + "=" * 70)
print("TESTE 1: MINIMO DE |Im(K_q K_{1-q})| NO RETANGULO")
print("=" * 70)

q_list = [2, 3, 5, 10, 100]
sigma_vals = np.linspace(0.0, 1.0, 41)
t_vals = np.linspace(0.0, 10.0, 41)

for q in q_list:
    min_imag = 1e10
    min_sigma = 0
    min_t = 0
    for sigma in sigma_vals:
        for t in t_vals:
            s = mp.mpc(sigma, t)
            im = imag_simetria(s, q)
            if abs(im) < abs(min_imag):
                min_imag = im
                min_sigma = sigma
                min_t = t
    print(f"q = {q:3d}: min |Im| = {float(abs(min_imag)):.2e} em sigma={min_sigma:.2f}, t={min_t:.2f}")

# ============================================================
# TESTE 2: Varredura vertical em sigma=0.5 para varios t
# ============================================================
print("\n" + "=" * 70)
print("TESTE 2: |Im(K_q K_{1-q})| AO LONGO DE sigma=0.5")
print("=" * 70)

for q in q_list:
    max_imag = 0
    for t in np.linspace(0, 20, 101):
        s = mp.mpc(0.5, t)
        im = imag_simetria(s, q)
        if abs(im) > abs(max_imag):
            max_imag = im
    print(f"q = {q:3d}: max |Im| em sigma=0.5 = {float(abs(max_imag)):.2e}")

# ============================================================
# TESTE 3: Linhas excepcionais t = n*pi/ln(q)
# ============================================================
print("\n" + "=" * 70)
print("TESTE 3: LINHAS EXCEPCIONAIS t = n*pi/ln(q)")
print("  (Im deve ser 0 para qualquer sigma nestas linhas)")
print("=" * 70)

for q in q_list[0:3]:
    print(f"\n--- q = {q} ---")
    for n in [1, 2, 3, 5]:
        t_exc = n * mp.pi / mp.log(q)
        for sigma in [0.2, 0.5, 0.8]:
            s = mp.mpc(sigma, t_exc)
            im = imag_simetria(s, q)
            print(f"  n={n}, t={float(t_exc):.4f}, sigma={sigma:.1f}: Im = {float(im):.2e}")

# ============================================================
# TESTE 4: Verificar que |Im| eh minima em sigma=0.5
#   Para varios t fixos, varrer sigma
# ============================================================
print("\n" + "=" * 70)
print("TESTE 4: MINIMO EM SIGMA PARA t FIXO (q=2)")
print("=" * 70)

q = 2
for t_fixo in [0, 1, 3.1416, 5]:
    min_im = 1e10
    min_sig = 0
    for sigma in np.linspace(0, 1, 101):
        s = mp.mpc(sigma, t_fixo)
        im = abs(imag_simetria(s, q))
        if im < min_im:
            min_im = im
            min_sig = sigma
    print(f"t = {t_fixo:.4f}: min |Im| = {float(min_im):.2e} em sigma = {min_sig:.3f}")

# ============================================================
# TESTE 5: Para q complexo
# ============================================================
print("\n" + "=" * 70)
print("TESTE 5: q COMPLEXO (q = 2*exp(i*theta))")
print("=" * 70)

for theta in [0.1, 0.5, 1.0, 1.5]:
    q_comp = 2 * mp.e**(1j * theta)
    min_im = 1e10
    min_sig = 0
    min_t = 0
    for sigma in np.linspace(0, 1, 21):
        for t in np.linspace(0, 5, 21):
            s = mp.mpc(sigma, t)
            im = abs(imag_simetria(s, q_comp))
            if im < min_im:
                min_im = im
                min_sig = sigma
                min_t = t
    print(f"theta = {theta:.2f}: min |Im| = {float(min_im):.2e} em sigma={min_sig:.2f}, t={min_t:.2f}")

print("\n" + "=" * 70)
print("RESUMO")
print("=" * 70)
print("1. Para q real > 0, q != 1:")
print("   - Im(K_q K_{1-q}) = 0 em sigma=0.5 (qualquer t)")
print("   - Im(K_q K_{1-q}) = 0 em t = n*pi/ln(q) (qualquer sigma)")
print("2. A linha sigma=0.5 e UNIVERSAL para qualquer base real")
print("3. Para q complexo, o minimo NAO esta necessariamente em sigma=0.5")
