# The Valence Layer Algorithm — Resultados Numéricos

## Maior primo suportado (64-bit)

```
9.223.372.036.854.775.783
```
Gap até o primo anterior: 140 (1×120 + 1×20). Apenas 24 unidades abaixo do limite 2⁶³−1.

## Gaps máximos encontrados

| Range | Primos | Gap máx | Decomposição (gulosa) | Termos |
|-------|--------|---------|----------------------|--------|
| [10¹², 10¹²+10⁷] | 361.726 | 322 | 2×120 + 1×64 + 1×10 + 1×6 + 1×2 | 5 |
| [10¹⁵, 10¹⁵+10⁶] | 28.845 | 320 | 2×120 + 1×64 + 1×16 | 4 |
| [10¹⁵, 10¹⁵+10⁸] | ~2.560M+ | 490 | 4×120 + 1×10 | 5 |

## Decomposições exemplo

| Gap | Guloso | Termos | Variado | Termos |
|-----|--------|--------|---------|--------|
| 2 | 1×2 | 1 | 1×2 | 1 |
| 4 | 1×4 | 1 | 1×4 | 1 |
| 6 | 1×6 | 1 | 1×6 | 1 |
| 10 | 1×10 | 1 | 1×10 | 1 |
| 20 | 1×20 | 1 | 1×20 | 1 |
| 34 | 1×34 | 1 | 1×34 | 1 |
| 64 | 1×64 | 1 | 1×64 | 1 |
| 114 | 1×64 + 1×34 + 1×16 | 3 | 1×64 + 1×34 + 1×16 | 3 |
| 140 | 1×120 + 1×20 | 2 | 1×120 + 1×20 | 2 |
| 320 | 2×120 + 1×64 + 1×16 | 4 | 1×120+1×64+1×34+1×32+1×30+1×28+1×12 | 7 |
| 322 | 2×120 + 1×64 + 1×10 + 1×6 + 1×2 | 5 | — | — |
| 490 | 4×120 + 1×10 | 5 | — | — |

## Tool set completo (16 ferramentas)

```
[120, 64, 34, 32, 30, 28, 24, 20, 18, 16, 12, 10, 8, 6, 4, 2]
```

Soma máxima com cada tool usada no máximo 1 vez: 428.

## Primos notáveis

| Descrição | Valor |
|-----------|-------|
| Primeiro primo | 2 |
| Último primo < 10⁹ | 999.999.937 |
| π(10⁹) | 50.847.534 |
| π(10¹⁰) | 455.052.511 |
| Maior primo < 2⁶³ | 9.223.372.036.854.775.783 |

## Algoritmo de primalidade

Miller-Rabin determinístico com 7 bases para n < 2⁶⁴.
Multiplicação modular 128-bit segura (sem overflow).
