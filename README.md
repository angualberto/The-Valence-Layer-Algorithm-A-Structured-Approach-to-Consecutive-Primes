# The Valence Layer Algorithm: A Structured Approach to Consecutive Primes

## Algoritmo da Camada de Valencia

Uma abordagem estruturada para decomposição de gaps entre primos consecutivos usando um conjunto fixo de "ferramentas" (valences).

### Conceito

Todo gap entre primos consecutivos (exceto 3−2=1) é **par**.  
Portanto, qualquer gap pode ser decomposto como soma de números pares.

A **Caixa de Ferramentas** é um conjunto flexível de números pares que cresce conforme necessário.  
O conjunto atual — otimizado para gaps até ~10⁴ — é:

```
F = {1000, 500, 200, 120, 64, 34, 32, 30, 28, 24, 20, 18, 16, 12, 10, 8, 6, 4, 2}
```

A decomposição gulosa (maior primeiro) sempre encontra uma representação exata  
porque **o 2 está sempre presente** — qualquer par é múltiplo de 2.  
O modo `-variado` usa cada ferramenta no máximo uma vez, maximizando a diversidade de termos.

Exemplo: gap 114 = 64 + 34 + 16 (3 termos, modo variado: 64+34+16).

A caixa pode ser expandida com mais pares para gaps maiores — a única exigência  
é que o **2** esteja sempre presente, pois ele é o bloco fundamental  
(2 = 1+1, a primeira decomposição).

### Estrutura do Projeto

```
├── src/
│   ├── primos_mod.f90          # Módulo (Miller-Rabin 64-bit + gap decomposition)
│   ├── miller_rabin_128.f90    # Miller-Rabin 128-bit (12 bases)
│   ├── primos_valencia.f90     # CLI principal
│   └── crivo_segmentado.f90    # Crivo segmentado (π(N) até 10¹⁰+)
├── demo/
│   ├── verifica_gap.f90        # Verificação de gap específico
│   ├── verifica_gap_grande.f90 # Gap do maior primo 64-bit
│   ├── busca_gap_rapido.f90    # Busca rápida de gaps grandes
│   ├── agle_distribution.f90   # Integração AGLE (análise estatística)
│   ├── maior_primo.f90         # Maior primo < 2⁶³
│   └── test_mr128.f90          # Teste do Miller-Rabin 128-bit
├── docs/
│   ├── primalidade.md          # Documentação do teste de primalidade
│   └── resultados.md           # Resultados numéricos
├── results/
│   └── primeiros_30_primos.txt # Sample output
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
```

### Compilação

```bash
mingw32-make          # compila src/
mingw32-make demo     # compila também demo/
```

Requer **gfortran** (GCC ≥ 14, mingw-w64).

### Uso

#### Listar primos com decomposição de gaps

```bash
bin/primos_valencia -inicio 2 -quantos 10
```

#### Buscar o maior gap num range

```bash
bin/primos_valencia -inicio 1e15 -ate 1e15+1e6 -maxgap
bin/primos_valencia -inicio 1e12 -quantos 100000 -maxgap
```

#### Decomposição variada (mais termos)

```bash
bin/primos_valencia -inicio 1e12 -ate 1e12+1e6 -maxgap -variado
```

Opções:
- `-inicio N` — primo inicial (busca o primeiro ≥ N)
- `-quantos N` — quantos primos gerar
- `-ate N` — gerar primos até N
- `-arquivo NOME` — arquivo de saída
- `-silencioso` — exibir apenas no arquivo
- `-maxgap` — modo busca do maior gap (requer -ate ou -quantos)
- `-variado` — decomposição com máximo de ferramentas diferentes

#### Contar primos via crivo segmentado

```bash
echo 1000000000 | bin/crivo_segmentado
```

#### Maior primo suportado (64-bit)

```bash
bin/maior_primo
```

### Exemplo de saída

```
bin/primos_valencia -inicio 1e15 -ate 1e15+1e6 -maxgap -variado

Primos: 28845
Gap maximo: 320
  em 1000000000991447 -> 1000000000991767
Termos (guloso): 4
  2x120 1x64 0x... 1x16 ...
Termos (variado): 7
  1x120 1x64 1x34 1x32 1x30 1x28 0x... 1x12 ...
```

### Teste de Primalidade — Miller-Rabin

O Algoritmo da Camada de Valência depende de um teste de primalidade rápido e determinístico. O Miller-Rabin é a escolha ideal.

**64 bits** (`primos_mod.f90`):
- 7 bases determinísticas para `n < 2⁶⁴`
- `mul_mod` com inteiro **128-bit intermediário** para evitar overflow
- Usado por `proximo_primo` para validar candidatos a primo

**128 bits** (`miller_rabin_128.f90`):
- 12 bases determinísticas para `n < 2¹²⁸`
- `add_mod` seguro sem overflow via subtração condicional
- `mul_mod` por duplicação (Russian peasant)

```fortran
! O coração do algoritmo: is_prime usado dentro de proximo_primo
subroutine proximo_primo(start_number, next_prime, gap, ...)
  do
     candidato = candidato + 1
     if (is_prime(candidato)) exit   ! Miller-Rabin 64-bit
  end do
end subroutine
```

Veja [`docs/primalidade.md`](docs/primalidade.md) para detalhes completos.

### Large Prime Showcase

| Prime | Size | Found By | Gap Decomposition |
|-------|------|----------|-------------------|
| `9,223,372,036,854,775,783` | 64-bit (19 digits) | Fortran | 140 = 120 + 20 |
| `170,141,183,460,469,231,731,687,303,715,884,105,727` | 128-bit (39 digits) = M127 | Fortran (Mersenne) | — |
| `p ≈ 10^{4999} + δ` | 5000 digits | C+GMP (1039 s) | Gap 9978 = 120×83 + ... |

The 64-bit prime `2⁶³−25` is the largest prime below `2⁶³`. The 128-bit
prime `M127 = 2¹²⁷−1` is a Mersenne prime. The 5000-digit prime was
produced by the C+GMP arbitrary-precision implementation
(`src/valencia_gmp.c`).

### Licença

MIT License. Veja [LICENSE](LICENSE).
