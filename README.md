# The Valence Layer Algorithm: A Structured Approach to Consecutive Primes

## Algoritmo da Camada de Valencia

Uma abordagem estruturada para decomposição de gaps entre primos consecutivos usando um conjunto fixo de "ferramentas" (valences).

### Conceito

Todo gap par entre primos consecutivos pode ser decomposto como soma de elementos do conjunto de ferramentas:

```
F = {120, 64, 34, 32, 30, 28, 24, 20, 18, 16, 12, 10, 8, 6, 4, 2}
```

A decomposição gulosa (maior primeiro) sempre encontra uma representação exata. O modo `-variado` usa cada ferramenta no máximo uma vez, maximizando a diversidade de termos.

Exemplo: gap 114 = 64 + 34 + 16 (3 termos, modo variado: 64+34+16).

### Estrutura do Projeto

```
├── src/
│   ├── primos_mod.f90          # Módulo (Miller-Rabin + gap decomposition)
│   ├── primos_valencia.f90     # CLI principal
│   └── crivo_segmentado.f90    # Crivo segmentado (π(N) até 10¹⁰+)
├── demo/
│   ├── verifica_gap.f90        # Verificação de gap específico
│   ├── verifica_gap_grande.f90 # Gap do maior primo 64-bit
│   ├── busca_gap_rapido.f90    # Busca rápida de gaps grandes
│   ├── agle_distribution.f90   # Integração AGLE (análise estatística)
│   └── maior_primo.f90         # Maior primo < 2⁶³
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

### Verificação de Primalidade

O módulo `primos_mod` usa:
- **Miller-Rabin determinístico** com 7 bases para n < 2⁶⁴
- **Multiplicação modular segura** com inteiro 128-bit (evita overflow)
- **Decomposição gulosa** e **variada** de gaps no conjunto de 16 ferramentas

### Maior primo 64-bit

```
9.223.372.036.854.775.783
```

Gap até o anterior: 140 (1×120 + 1×20). Penúltimo primo antes do limite de 2⁶³.

### Licença

MIT License. Veja [LICENSE](LICENSE).
