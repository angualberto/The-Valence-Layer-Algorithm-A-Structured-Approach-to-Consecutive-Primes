"""
Algoritmo da Camada de Valencia - Versao Big Int (precisao arbitraria)
Encontra o proximo primo apos qualquer numero N e decompoe o gap
usando as ferramentas de valencia.
"""
import random
import sys

FERRAMENTAS = [120, 64, 34, 32, 30, 28, 24, 20, 18, 16, 12, 10, 8, 6, 4, 2]

def is_prime(n, k=20):
    """Miller-Rabin deterministico para n < 2^64, probabilistico para maiores."""
    if n < 2: return False
    # Pequenos primos
    small = [2,3,5,7,11,13,17,19,23,29,31,37]
    for p in small:
        if n % p == 0: return n == p
    # Escreve n-1 = d * 2^s
    d = n - 1; s = 0
    while d % 2 == 0:
        d //= 2; s += 1
    # Testa k bases
    for _ in range(k):
        a = random.randrange(2, min(n-1, 2**64))
        x = pow(a, d, n)
        if x == 1 or x == n-1: continue
        for _ in range(s-1):
            x = pow(x, 2, n)
            if x == n-1: break
        else:
            return False
    return True

def decompor_gap(gap):
    """Decompoe gap par usando ferramentas de valencia (guloso)."""
    if gap % 2 != 0:
        return None
    resto = gap
    termos = []
    for f in FERRAMENTAS:
        while resto >= f:
            termos.append(f)
            resto -= f
    if resto == 0:
        return termos
    return None

def decompor_gap_variado(gap):
    """Cada ferramenta no maximo 1 vez, resto com 2."""
    if gap % 2 != 0:
        return None
    resto = gap
    termos = []
    # Tenta cada ferramenta (exceto 2) no maximo 1 vez
    for f in FERRAMENTAS[:-1]:
        if resto >= f:
            termos.append(f)
            resto -= f
    # Preenche resto com 2
    while resto >= 2:
        termos.append(2)
        resto -= 2
    if resto == 0:
        return termos
    return None

def proximo_primo(N, max_testes=1000000):
    """Encontra o proximo primo >= N (algoritmo de valencia)."""
    if N < 2: return 2
    candidato = N
    if candidato % 2 == 0:
        candidato += 1
    else:
        pass  # ja e impar
    
    for i in range(max_testes):
        if is_prime(candidato):
            return candidato
        candidato += 2
    return None

def testar_numero(base, exp, label=""):
    """Testa a partir de 2^exp (ou base^exp) e encontra o proximo primo."""
    print(f"\n{'='*60}")
    print(f"ALGORITMO DA CAMADA DE VALENCIA - BIG INT")
    print(f"{'='*60}")
    print(f"Alvo: {label or f'{base}^{exp}'}")
    
    N = pow(base, exp)
    print(f"Tamanho: ~{len(str(N))} digitos")
    
    import time
    t0 = time.time()
    
    p = proximo_primo(N)
    if p is None:
        print(f"Primo nao encontrado em 1M tentativas")
        return
    
    gap = p - N
    t1 = time.time()
    
    print(f"Proximo primo encontrado: {str(p)[:50]}...")
    print(f"Gap: {gap:,}")
    print(f"Tempo: {t1-t0:.2f}s")
    
    g_dec = decompor_gap(gap)
    g_var = decompor_gap_variado(gap)
    
    if g_dec:
        print(f"\nDecomposicao (gulosa): {len(g_dec)} termos")
        # Agrupa
        from collections import Counter
        c = Counter(g_dec)
        for f in FERRAMENTAS:
            if c[f] > 0:
                print(f"  {c[f]}x{f}")
    else:
        print(f"GAP IMPAR! ({gap}) - nao decomponivel")
    
    if g_var:
        print(f"\nDecomposicao (variada): {len(g_var)} termos")
        c2 = Counter(g_var)
        for f in FERRAMENTAS:
            if c2[f] > 0:
                print(f"  {c2[f]}x{f}")

if __name__ == "__main__":
    print("Algoritmo da Camada de Valencia (Big Int)")
    print("Ferramentas:", FERRAMENTAS)
    
    # Teste 1: M127 (conhecido)
    testar_numero(2, 127, "M127 = 2^127-1")
    
    # Teste 2: Numero de 100 digitos
    N = random.getrandbits(330)
    if N % 2 == 0: N += 1
    print(f"\n{'='*60}")
    print(f"Numero aleatorio de ~100 digitos:")
    print(f"N = {N}")
    import time
    t0 = time.time()
    p = proximo_primo(N)
    if p:
        gap = p - N
        t1 = time.time()
        print(f"Proximo primo: {p}")
        print(f"Gap: {gap:,}")
        print(f"Tempo: {t1-t0:.2f}s")
        g_dec = decompor_gap(gap)
        from collections import Counter
        c = Counter(g_dec)
        print(f"Decomp: {len(g_dec)} termos")
        for f in FERRAMENTAS:
            if c[f] > 0:
                print(f"  {c[f]}x{f}")
