# Verificação de Primalidade — Algoritmo da Camada de Valencia

## Miller-Rabin Determinístico

O módulo `primos_mod.f90` implementa o teste de primalidade de **Miller-Rabin determinístico** para números de até 64 bits (`n < 2⁶⁴`).

### Bases determinísticas (64 bits)

```
[2, 325, 9375, 28178, 450775, 9780504, 1795265022]
```

Este conjunto (Sinclair, 2011) é correto para **todo** `n < 2⁶⁴`. Nenhum composto abaixo desse limite passa no teste.

### Algoritmo

```
is_prime(n):
  1. Se n ≤ 1           → FALSO
  2. Se n = 2 ou n = 3  → VERDADEIRO
  3. Divisores pequenos: testar primos ≤ 29
  4. Escrever n-1 = d × 2ˢ  (d ímpar)
  5. Para cada base a ∈ BASES:
     a. x = aᵈ mod n
     b. Se x = 1 ou x = n-1 → próximo
     c. Repetir s-1 vezes: x = x² mod n
     d. Se x ≠ n-1 → FALSO
  6. → VERDADEIRO
```

### Casos especiais tratados

| Condição | Resultado | Razão |
|----------|-----------|-------|
| n ≤ 1 | Composto | Definição |
| n = 2, 3 | Primo | Casos base |
| n par > 2 | Composto | Divisível por 2 |
| n múltiplo de 3,5,7,11,13,17,19,23,29 | Composto | Eliminação rápida |

### Implementação em Fortran

O módulo `primos_mod` contém:

- `mul_mod(a, b, m)` — multiplicação modular com inteiro **128-bit** intermediário
  ```fortran
  integer(i16) :: a16, b16, m16
  a16 = a; b16 = b; m16 = m
  res = int(mod(a16 * b16, m16), i8)
  ```
- `pow_mod(a, b, m)` — exponenciação modular O(log b)
- `is_prime(n)` — wrapper completo com as 7 bases

### Integração com o Algoritmo da Camada de Valência

O `is_prime` é usado diretamente por `proximo_primo`:

```fortran
subroutine proximo_primo(start_number, next_prime, gap, contagens, n_terms)
  ! ...
  candidato = start_number
  do
     candidato = candidato + 1
     if (is_prime(candidato)) exit   ! <-- Miller-Rabin aqui
  end do
  ! ...
end subroutine
```

Também é usado pelo crivo segmentado e pelo `preencher_gap` indiretamente (gaps pares são sempre decomponíveis, mas a primalidade dos candidatos é verificada via Miller-Rabin).

### Limitação: n < 2⁶⁴

O tipo `integer(i8)` no Fortran é **signed 64-bit**:

```
Intervalo: [−2⁶³, 2⁶³−1] ≈ [−9,22×10¹⁸, 9,22×10¹⁸]
Maior primo testável: 9.223.372.036.854.775.783 (gap = 24 até 2⁶³−1)
```

### Extensão para 128 bits

O módulo `miller_rabin_128` (em `src/miller_rabin_128.f90`) estende o teste para `n < 2¹²⁸` usando:

- **12 bases determinísticas**: [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37]
- **Multiplicação modular por duplicação** (`add_mod` seguro sem overflow)
- `integer(i16)` = `selected_int_kind(33)` (128-bit signed)

```fortran
! Adição modular sem overflow
function add_mod(a, b, m) result(res)
  if (a > m - b) then
    res = a - (m - b)   ! a + b - m
  else
    res = a + b
  end if
end function
```

### Referências

- Jim Sinclair, "Deterministic Miller-Rabin for n < 2^64" (2011)
- Bases para 2⁶⁴ verificadas por Iain McDonald (2020)
- Bases para 2¹²⁸: Jim Sinclair / PrimeFac (2011)
