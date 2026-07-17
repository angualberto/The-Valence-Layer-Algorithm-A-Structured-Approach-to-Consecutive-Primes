"""
Analisa a cauda (ultimos digitos) de primos consecutivos
gerados pelo Algoritmo da Camada de Valencia (C+GMP).
"""
import subprocess
import sys
import time
from collections import Counter

VALENCIA = r"valencia_gmp.exe"

def encontrar_proximo_primo(N):
    """Retorna (proximo_primo, gap) usando valencia_gmp.exe"""
    try:
        r = subprocess.run([VALENCIA, str(N)], capture_output=True, text=True, timeout=300)
        # Parse output lines
        lines = r.stdout.strip().split('\n')
        primo_str = None
        gap_str = None
        for l in lines:
            if l.startswith("Prox primo:"):
                # Extract the number
                primo_str = l.split(":")[1].strip().split()[0]
            if l.startswith("Gap:"):
                gap_str = l.split(":")[1].strip().split()[0]
        if primo_str:
            return int(primo_str), int(gap_str) if gap_str else None
        return None, None
    except Exception as e:
        print(f"Erro: {e}")
        return None, None

def gerar_primos(inicio, quantidade):
    """Gera N primos consecutivos a partir de inicio."""
    primos = []
    atual = inicio
    for i in range(quantidade):
        p, gap = encontrar_proximo_primo(atual)
        if p is None:
            print(f"Falhou em n={atual}")
            break
        primos.append(p)
        atual = p + 1  # proximo candidato depois do primo
        if (i+1) % 100 == 0:
            print(f"  {i+1}/{quantidade} primos gerados (ultimo={str(p)[:20]}...)")
    return primos

def analisar_cauda(primos):
    """Conta ultimos digitos."""
    c = Counter(p % 10 for p in primos)
    total = len(primos)
    print(f"\nTotal de primos: {total}")
    print(f"{'Digito':>8} | {'Contagem':>10} | {'Proporcao':>10} | {'Esperado':>10}")
    print("-" * 45)
    for d in [1, 3, 7, 9]:
        cnt = c[d]
        prop = 100 * cnt / total
        print(f"{d:>8} | {cnt:>10} | {prop:>8.2f}% | {'25.00%':>10}")
    print(f"{'outros':>8} | {c[0]+c[2]+c[4]+c[5]+c[6]+c[8]:>10}")

if __name__ == "__main__":
    print("="*60)
    print("ANALISE DA CAUDA (ULTIMOS DIGITOS) DOS PRIMOS")
    print("Algoritmo da Camada de Valencia - C+GMP")
    print("="*60)
    
    # Teste 1: primos pequenos (ate 10^6)
    print("\n--- Teste 1: Primos < 10^6 ---")
    primos_peq = []
    for n in range(2, 1000000):
        p, _ = encontrar_proximo_primo(n)
        if p and p < 1000000:
            primos_peq.append(p)
            n = p + 1
        else:
            break
    analisar_cauda(primos_peq)
    
    # Teste 2: 1000 primos perto de 10^100
    print("\n--- Teste 2: 1000 primos perto de 10^100 ---")
    t0 = time.time()
    primos_grandes = gerar_primos(10**100, 1000)
    t1 = time.time()
    print(f"Tempo: {t1-t0:.1f}s")
    analisar_cauda(primos_grandes)
    
    if len(primos_grandes) == 1000:
        # Chebyshev bias: verifica se 3+7 > 1+9
        c = Counter(p % 10 for p in primos_grandes)
        bias_37 = c[3] + c[7]
        bias_19 = c[1] + c[9]
        print(f"\nVies de Chebyshev: 3+7={bias_37}, 1+9={bias_19}")
        print(f"  Diferenca: {bias_37 - bias_19} ({'+37' if bias_37 > bias_19 else '+19'})")
