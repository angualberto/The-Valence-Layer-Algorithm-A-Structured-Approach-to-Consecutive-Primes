# The Valence Layer Algorithm: A Structured Approach to Consecutive Primes

## Algoritmo da Camada de Valencia

Uma abordagem estruturada para decomposição de gaps entre primos consecutivos usando um conjunto fixo de "ferramentas" (valences).

### Conceito

Todo gap par entre primos consecutivos pode ser decomposto como soma de elementos do conjunto de ferramentas:

```
F = {120, 64, 34, 20, 10, 6, 4, 2}
```

A decomposição gulosa (maior primeiro) sempre encontra uma representação exata. O número de termos da decomposição é a **valência** do gap.

Exemplo: gap 114 = 64 + 34 + 10 + 6 (4 termos, valência 4).

### Estrutura do Projeto

```
├── src/
│   ├── primos_mod.f90          # Módulo principal (Miller-Rabin + gap decomposition)
│   ├── primos_valencia.f90     # CLI: gera primos com decomposição de gaps
│   └── crivo_segmentado.f90    # Crivo segmentado para π(N) (até 10¹⁰+)
├── demo/
│   └── verifica_gap.f90        # Demo de verificação de gap
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
```

### Compilação

```bash
make          # compila tudo (primos_valencia + crivo_segmentado)
make demo     # compila também o demo verifica_gap
make clean    # limpa binários
```

Requer **gfortran** (GCC ≥ 14 recomendado).

### Uso

#### Listar primos com decomposição de gaps

```bash
./bin/primos_valencia -inicio 2 -quantos 10
```

Opções:
- `-inicio N` — primo inicial (busca o primeiro ≥ N)
- `-quantos N` — quantos primos gerar
- `-ate N` — gerar primos até N (alternativo a -quantos)
- `-arquivo NOME` — arquivo de saída
- `-silencioso` — exibir apenas no arquivo

#### Contar primos via crivo segmentado

```bash
echo 1000000000 | ./bin/crivo_segmentado
```

#### Verificar gap específico

```bash
./bin/verifica_gap
```

### Saída do Algoritmo de Valencia

Cada primo é seguido pela decomposição do gap até o próximo:

```
492007
492013
  Gap: 6  Termos: 1
  Ferramentas: 1x6
492017
  Gap: 4  Termos: 1
  Ferramentas: 1x4
...
492113
  Gap: 114  Termos: 4
  Ferramentas: 1x64 1x34 1x10 1x6
492227
```

### Verificação de Primalidade

O módulo `primos_mod` usa:
- **Miller-Rabin determinístico** com 7 bases para n < 2⁶⁴
- **Multiplicação modular segura** com inteiro 128-bit intermediário (evita overflow)
- **Decomposição gulosa** dos gaps no conjunto de ferramentas fixo

### Licença

MIT License. Veja [LICENSE](LICENSE).
