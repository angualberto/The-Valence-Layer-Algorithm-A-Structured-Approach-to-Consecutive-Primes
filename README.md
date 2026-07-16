# The Valence Layer Algorithm — RSA Gap Inversion Attack

Algoritmo de inversão de gap para quebra de chaves RSA com fator compartilhado, utilizando busca em dicionário de gaps pares, bigint de precisão mista e alinhamento de cache L1.

Quebra RSA-2048 em **0.90 ms** (710 passos), ou **6.7 µs** com alinhamento `alignas(64)`.

## Passo a passo

### 1. Clonar

```bash
git clone https://github.com/angualberto/The-Valence-Layer-Algorithm-A-Structured-Approach-to-Consecutive-Primes.git
cd The-Valence-Layer-Algorithm-A-Structured-Approach-to-Consecutive-Primes
```

### 2. Teste rápido (Python)

Gera um par RSA com primos de 256 bits e quebra instantaneamente:

```bash
python quebra_rsa_gap.py --gera 256
```

Saída esperada:
```
p gerado  = 0x9248...aac7
gap_q     = 52
=== RSA QUEBRADO em 0.01ms ===
passos   = 26
```

### 3. Escalar para chaves maiores

```bash
python quebra_rsa_gap.py --gera 512    # RSA-512  (0.02ms)
python quebra_rsa_gap.py --gera 1024   # RSA-1024 (0.07ms)
python quebra_rsa_gap.py --gera 2048   # RSA-2048 (0.90ms)
```

O `max_gap` escala dinamicamente com o tamanho do primo (`max(500, prime_bits * 2)`).

### 4. Ataque manual (dois módulos hex)

```bash
python quebra_rsa_gap.py 0x9e4b...c2cf 0x9e4b...fd695
```

### 5. Compilar e rodar os testes C++

```bash
g++ -O3 -march=native -std=c++11 teste_simetria_gap.cpp -o teste_simetria_gap
./teste_simetria_gap
```

Testa simetria de Fermat (`N % 4`), dicionário de gaps e extração de fator composto par.

```bash
g++ -O3 -march=native -std=c++11 teste_bloco_alinhado.cpp -o teste_bloco_alinhado
./teste_bloco_alinhado
```

Benchmark do alinhamento `alignas(64)` com dados reais do RSA-2048 (~6.7 µs).

### 6. Ataque RSA-256 em C (bigint limbs)

```bash
g++ -O3 -std=c++11 ataque_rsa256.c -o ataque_rsa256
ataque_rsa256 N1_hex N2_hex
```

### 7. Compilar o artigo acadêmico

Requer `pdflatex` com TikZ e PGFPlots:

```bash
pdflatex algoritimo_pt.tex   # Português (25 pág.)
pdflatex algoritimo_en.tex   # Inglês    (22 pág.)
```

## Resultados

| Bits($p$) | Bits($N$) | gap$_q$ | Passos | max_gap | Tempo |
|-----------|-----------|---------|--------|---------|-------|
| 256 | 512 | 52 | 26 | 500 | <0.01 ms |
| 512 | 1024 | 30 | 15 | 1024 | 0.02 ms |
| 1024 | 2047 | 66 | 33 | 2048 | 0.07 ms |
| 2048 | 4096 | 1420 | 710 | 4096 | 0.90 ms |

Com alinhamento `alignas(64)` + cache L1: **6.7 µs** (speedup de **134×**).

## Hardware dos testes

Intel Core i3-7020U @ 2.30 GHz (2 cores, 4 threads), L2 512 KB, L3 3 MB, 16 GB DDR4, Windows, GCC (MinGW) `-O3 -march=native`.

## Arquivos principais

| Arquivo | Descrição |
|---------|-----------|
| `quebra_rsa_gap.py` | Ataque por inversão de gap (Python, dinâmico) |
| `ataque_rsa256.c` | Ataque com bigint de precisão mista (C) |
| `bigint_min.h` | Biblioteca de limbs u64 (até 4096 bits) |
| `teste_simetria_gap.cpp` | Valida simetria de Fermat + dicionário |
| `teste_bloco_alinhado.cpp` | Benchmark alignas(64) com dados reais |
| `algoritimo_pt.tex` / `.pdf` | Artigo acadêmico (Português, 25 pág.) |
| `algoritimo_en.tex` / `.pdf` | Artigo acadêmico (Inglês, 22 pág.) |

## Licença

MIT License.
