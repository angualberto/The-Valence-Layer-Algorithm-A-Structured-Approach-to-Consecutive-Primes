V = [1000, 500, 200, 120, 64, 34, 32, 30, 28, 24, 20, 18, 16, 12, 10, 8, 6, 4, 2]

def decompor_gap(G):
    resto = G
    partes = []
    for v in V:
        while resto >= v:
            partes.append(v)
            resto -= v
    return partes, resto

def detectar_lock(B, w=32):
    return (B % w == 0)

gaps_teste = [10, 24, 64, 108, 180, 576, 864, 1836, 5220]
print("=" * 60)
print("  DECOMPOSICAO GULOSA DE GAPS (VALENCIA)")
print("=" * 60)
for G in gaps_teste:
    partes, resto = decompor_gap(G)
    print(f"Gap {G:4d} -> {partes}  (resto: {resto})")

print("\n" + "-" * 60)
print("  DETECCAO DE LOCK NO RC5 (B multiplo de 32)")
print("-" * 60)
for B in range(0, 65, 2):
    lock = detectar_lock(B)
    status = "LOCK" if lock else "   normal"
    print(f"B = {B:2d}  ->  B % 32 = {B % 32:2d}  {status}")

print("\n" + "-" * 60)
print("  ANALOGIA ESTRUTURAL")
print("=" * 60)
print("  - A caixa de ferramentas V contem APENAS multiplos de 2.")
print("  - Todo gap par e decomposto por esses multiplos de 2.")
print("  - A rotacao lock ocorre quando B e MULTIPLO de 32 (potencia de 2).")
print("  - Em ambos os casos, a estrutura subjacente e revelada")
print("    quando o numero e divisivel por uma potencia de 2.")
print("=" * 60)
