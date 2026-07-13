# Verificação de Primalidade no Algoritmo da Camada de Valencia

## Método: Miller-Rabin Determinístico

O módulo `primos_mod.f90` implementa o teste de primalidade de **Miller-Rabin determinístico** para números de até 64 bits (n < 2⁶⁴).

### Bases utilizadas (7 bases)

```
[2, 325, 9375, 28178, 450775, 9780504, 1795265022]
```

Este conjunto é deterministicamente correto para todo n < 2⁶⁴ (resultado de Jim Sinclair, 2011). Nenhum número composto abaixo de 2⁶⁴ passa no teste com estas 7 bases.

### Algoritmo

```
1. Se n < 2 → composto
2. Se n ∈ {2,3,5,7,11,13,17,19,23,29} → primo
3. Se n é divisível por qualquer primo ≤ 29 → composto
4. Escrever n−1 = d × 2ˢ  (d ímpar)
5. Para cada base a:
   a. x = aᵈ mod n
   b. Se x = 1 ou x = n−1 → passar para próxima base
   c. Para r = 1 até s−1:
      - x = x² mod n
      - Se x = n−1 → sair do loop interno
   d. Se x ≠ n−1 → composto
6. Se passou todas as bases → primo
```

### Implementação

- `mul_mod(a, b, m)`: multiplicação modular segura usando inteiro **128-bit** intermediário (`integer(kind=16)`) para evitar overflow em `a × b` antes do `mod m`
- `pow_mod(a, b, m)`: exponenciação modular rápida O(log b) via quadratura repetida
- `is_prime(n)`: função principal, chama as anteriores com as 7 bases

### Limitação

O tipo `integer(i8)` no Fortran é **signed 64-bit**: intervalo [−2⁶³, 2⁶³−1] ≈ [−9,22×10¹⁸, 9,22×10¹⁸].

**Maior primo testável**: 9.223.372.036.854.775.783 (gap 24 abaixo de 2⁶³−1).

### Uso no programa

```fortran
use primos_mod

if (is_prime(n)) then
    print *, n, "eh primo"
else
    print *, n, "nao eh primo"
end if
```

### Linha de comando

```bash
# Verificar primalidade de um numero especifico:
# (usar o codigo fonte como referencia ou adaptar)

# Exemplo com o programa primos_valencia:
# O programa so processa numeros que sao primos,
# entao -inicio com um numero composto avanca ate o proximo primo
bin\primos_valencia.exe -inicio 9223372036854775783 -quantos 1 -silencioso
```

### Referências

- Jim Sinclair, "Deterministic Miller-Rabin for n < 2^64" (2011)
- Bases verificadas e confirmadas por Iain McDonald, 2020
