"""
Analise dos ultimos 2 digitos de primos gigantes (Ressonancia da Oitava)
Usa gmpy2 para geracao rapida de primos consecutivos.
"""
import gmpy2
import matplotlib.pyplot as plt
import numpy as np
from collections import Counter
import sys
import time

NUM_PRIMOS = 50000
DIGITOS = 200
START = gmpy2.mpz(10) ** DIGITOS

def gerar_primos(inicio, quantidade):
    primos = []
    p = gmpy2.next_prime(inicio)
    for i in range(quantidade):
        primos.append(p)
        p = gmpy2.next_prime(p + 1)
        if (i+1) % 10000 == 0:
            print(f"  {i+1}/{quantidade} gerados (ultimo={str(p)[:20]}...)")
    return primos

print(f"Gerando {NUM_PRIMOS} primos a partir de 10^{DIGITOS}...")
t0 = time.time()
primos = gerar_primos(START, NUM_PRIMOS)
t1 = time.time()
print(f"Prontos! ({t1-t0:.1f}s)")

ultimos_dois = [int(p % 100) for p in primos]
contagem = Counter(ultimos_dois)

terminacoes_validas = [i for i in range(100) if i % 2 != 0 and i % 5 != 0]
freqs = [contagem.get(t, 0) for t in terminacoes_validas]

x = np.arange(len(terminacoes_validas))
esperado = NUM_PRIMOS / 40

plt.figure(figsize=(18, 7))
bars = plt.bar(x, freqs, color='skyblue', edgecolor='navy', alpha=0.7)
plt.axhline(y=esperado, color='red', linestyle='--', linewidth=2,
            label=f'Esperado (1/40 = {esperado:.0f})')

# Colorir barras acima/abaixo da media
for i, (bar, freq) in enumerate(zip(bars, freqs)):
    if freq > esperado * 1.05:
        bar.set_color('orange')
    elif freq < esperado * 0.95:
        bar.set_color('lightcoral')

plt.xticks(x[::5], terminacoes_validas[::5], rotation=45, fontsize=9)
plt.xlabel('Ultimos 2 digitos (validos para primos > 5)', fontsize=12)
plt.ylabel('Frequencia', fontsize=12)
plt.title(f'Distribuicao dos ultimos 2 digitos em {NUM_PRIMOS} primos ≈ 10^{DIGITOS}', fontsize=14)
plt.legend()
plt.grid(axis='y', alpha=0.3)
plt.tight_layout()
plt.savefig('C:\\Users\\Andre\\Documents\\valence-layer-algorithm\\ultimos_2_digitos.png', dpi=300)
print(f"\nGrafico salvo: ultimos_2_digitos.png")

print("\n" + "="*60)
print("RESUMO DA RESSONANCIA DA OITAVA")
print("="*60)
print(f"Total de primos: {NUM_PRIMOS}")
print(f"Terminacoes validas (coprimas a 10): 40")
print(f"Esperado por classe: {esperado:.0f}")
desv_esp = np.sqrt(NUM_PRIMOS * (1/40) * (39/40))
print(f"Desvio padrao esperado: {desv_esp:.1f}")
print(f"Media observada: {np.mean(freqs):.1f}")
print(f"Desvio padrao observado: {np.std(freqs):.1f}")
print(f"Min: {min(freqs)} (terminacao {terminacoes_validas[freqs.index(min(freqs))]})")
print(f"Max: {max(freqs)} (terminacao {terminacoes_validas[freqs.index(max(freqs))]})")
print(f"Razao max/min: {max(freqs)/min(freqs):.3f}")
print("="*60)

# Teste qui-quadrado
chi2 = sum((f - esperado)**2 / esperado for f in freqs)
print(f"\nTeste Qui-Quadrado: χ² = {chi2:.1f} (gl=39)")
print(f"  p-valor ≈ {1 - sum(chi2**k * np.exp(-chi2/2) / (2**k * np.math.factorial(k//2) if k%2==0 else 2**((k-1)//2) * np.sqrt(chi2/np.pi) * np.math.factorial((k-1)//2)) for k in range(0, 40, 2)):.4f}")

print("\nInterpretacao:")
if chi2 < 50:
    print("  χ² < 50: Distribuicao consistente com uniformidade.")
else:
    print("  χ² grande: Possivel desvio da uniformidade (investigar).")
print("  (Esperado: pequenas flutuacoes aleatorias ao redor da media)")
