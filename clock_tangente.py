import numpy as np
import matplotlib.pyplot as plt

ln2 = np.log(2)
pi = np.pi

t = np.linspace(0, 20, 1000)
sigma = 0.75

cos_part = (2**sigma + 2**(1-sigma)) * np.cos(t * ln2)
sin_part = (2**sigma - 2**(1-sigma)) * np.sin(t * ln2)

phase = np.arctan2(sin_part, 3 - cos_part)
tangent = sin_part / (3 - cos_part)

T = 2 * pi / ln2
print(f"[CLOCK] Periodo fundamental T = {T:.4f}")
print(f"[CLOCK] Meio periodo (linhas excepcionais) = {T/2:.4f}")

fig, axes = plt.subplots(2, 1, figsize=(10, 8))

axes[0].plot(t, sin_part, label='Seno (Imag)', color='blue')
axes[0].plot(t, cos_part, label='Cosseno (Real)', color='red')
axes[0].axhline(0, color='black', linewidth=0.5)
for n in range(1, 10):
    t_ex = n * pi / ln2
    if t_ex < 20:
        axes[0].axvline(x=t_ex, color='gray', linestyle='--', alpha=0.5)
axes[0].set_title('Seno e Cosseno: zeros do seno marcam o clock')
axes[0].legend()
axes[0].grid(True, alpha=0.3)

axes[1].plot(t, tangent, label='Tangente', color='green')
axes[1].axhline(0, color='black', linewidth=0.5)
for n in range(1, 10):
    t_ex = n * pi / ln2
    if t_ex < 20:
        axes[1].axvline(x=t_ex, color='gray', linestyle='--', alpha=0.5)
axes[1].set_title('Tangente: assintotas e zeros delimitam o clock')
axes[1].legend()
axes[1].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('clock_tangente.png', dpi=150)
plt.close()
print("[OK] Grafico salvo como clock_tangente.png")
