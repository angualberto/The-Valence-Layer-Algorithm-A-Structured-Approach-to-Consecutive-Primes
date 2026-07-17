import numpy as np
import matplotlib.pyplot as plt

# ============================================================
# 1. SIMULAÇÃO DA ROTAÇÃO LOCK NO RC5
# ============================================================
def simular_rc5_lock(num_amostras=10000):
    b_values = np.random.randint(0, 32, num_amostras)
    locks = np.sum(b_values == 0)
    prob = locks / num_amostras
    print(f"[RC5] Simulacao com {num_amostras} amostras:")
    print(f"  - Rotacoes lock (B %% 32 == 0): {locks}")
    print(f"  - Probabilidade observada: {prob:.4f} (esperado: 1/32 = 0.03125)")
    return locks, prob

# ============================================================
# 2. SIMULAÇÃO DA FASE DA FUNÇÃO K(s)
# ============================================================
def simular_fase_zeta():
    t = np.linspace(0, 50, 1000)
    sigma = 0.75  # Exemplo: fora da linha critica
    # Im(K(s)K(1-s)) = (2^sigma - 2^(1-sigma)) * sin(t * ln 2)
    diff = 2**sigma - 2**(1 - sigma)
    imag = diff * np.sin(t * np.log(2))

    # Encontrar onde o seno zera (linhas excepcionais)
    # t = n * pi / ln(2)
    excecoes = np.arange(1, 50) * np.pi / np.log(2)
    excecoes = excecoes[excecoes < 50]

    # Plotar
    plt.figure(figsize=(10, 5))
    plt.plot(t, imag, label=r'$\Im(K(s)K(1-s))$ para $\sigma=0.75$', color='blue')
    plt.axhline(y=0, color='black', linestyle='--', linewidth=0.8)

    for ex in excecoes:
        plt.axvline(x=ex, color='red', linestyle=':', linewidth=1, alpha=0.6, label='Linha excepcional' if ex == excecoes[0] else "")

    plt.xlabel('t (parte imaginaria)')
    plt.ylabel('Imag(K(s)K(1-s))')
    plt.title('Simetria de Fase: Seno zero em t = npi/ln(2)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('fase_zeta_lock.png', dpi=150)
    plt.close()
    print("[Fase Zeta] Grafico salvo como 'fase_zeta_lock.png'")
    return excecoes

# ============================================================
# 3. EXECUTAR AMBOS OS TESTES
# ============================================================
if __name__ == "__main__":
    print("==================================================")
    print("  TESTE DA CONEXAO: ROTACAO LOCK + SIMETRIA DE FASE")
    print("==================================================\n")

    locks, prob = simular_rc5_lock(100000)

    print("\n" + "-" * 50 + "\n")

    excecoes = simular_fase_zeta()

    print("\n" + "-" * 50 + "\n")
    print("[CONCLUSAO]")
    print("  - No RC5: lock ocorre quando B e multiplo de 32 (seno zero).")
    print("  - Na RH: linhas excepcionais ocorrem quando t = npi/ln(2) (seno zero).")
    print("  - Em ambos, o seno zero revela a estrutura oculta:")
    print("      * RC5: carry (vazamento de bits).")
    print("      * RH: linha critica Re(s) = 1/2.")
    print("  - A probabilidade de lock no RC5 e 1/32. Nas linhas excepcionais,")
    print("    a probabilidade e 0 (um conjunto discreto).")
