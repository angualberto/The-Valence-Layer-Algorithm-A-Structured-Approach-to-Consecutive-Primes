INSTRUÇÕES PARA ENVIO AO
Journal of Integer Sequences (JIS)
https://cs.uwaterloo.ca/journals/JIS/

============================================================
 RESUMO DOS REQUISITOS (baseado no style guide v1.89)
============================================================

1. IDIOMA
   - Inglês americano (recognize, generalize, color, acknowledgment)
   - Se não for nativo, peça revisão de falante nativo

2. FORMATO LaTeX
   - Documentclass: \documentclass[12pt]{article}
   - Fonte: Computer Modern (padrão) — NÃO Times Roman
   - Um único arquivo .tex (não múltiplos arquivos)
   - NÃO usar pacotes proprietários (mdpi.cls, etc.)
   - NÃO usar tcilatex, amsart, Scientific Workplace, lyx, docx2latex
   - Remover pacotes e macros não utilizados
   - Remover TODOS os comentários (linhas com %)
   - Remover \date{} da página de título

3. ABSTRACT
   - 50–200 palavras
   - Tempo presente
   - Livre de símbolos e equações na medida do possível
   - Sem citações, sem referências a seções/teoremas/equações

4. SEÇÕES
   - Seções numeradas com \section, \subsection, etc.
   - Títulos capitalizados como frase normal
   - Introdução numerada obrigatória

5. TEOREMAS (usar amsthm)
   \theoremstyle{plain}
   \newtheorem{theorem}{Theorem}
   \newtheorem{proposition}[theorem]{Proposition}
   \newtheorem{corollary}[theorem]{Corollary}
   \theoremstyle{definition}
   \newtheorem{definition}[theorem]{Definition}
   \newtheorem{example}[theorem]{Example}
   \newtheorem{conjecture}[theorem]{Conjecture}
   \theoremstyle{remark}
   \newtheorem{remark}[theorem]{Remark}

6. PROVAS
   Usar \begin{proof}...\end{proof} (amsthm)
   NÃO modificar a aparência do ambiente proof

7. AGRADECIMENTOS
   Seção numerada ao final do artigo (antes da bibliografia)

8. FIGURAS E TABELAS
   - Centralizadas, caption abaixo
   - Numeradas, referenciadas por \label/\ref
   - Usar \usepackage{float} e opção H se posicionamento for crítico

9. BIBLIOGRAFIA
   - Usar \begin{thebibliography}{10}
   - Formato artigos:
       Autor, Título, {\it Revista Abrev.} \textbf{volume} (ano), páginas.
     Sem "vol.", sem "pp.", sem aspas no título
   - Formato livros:
       Autor, {\it Título}, Editora, ano.
   - Abreviações MR para nomes de revistas
   - Lista completa de autores (sem et al.)
   - Oxford comma (A, B, and C)

10. ABSTRACT — NÃO conter:
    - Citações bibliográficas
    - Referências a seções, teoremas ou equações do artigo
    - Equações numeradas

11. PALAVRAS-CHAVE E CLASSIFICAÇÃO AMS
    - Obrigatórios
    - keywords no singular, separadas por vírgula
    - Classificação AMS 2020 (primary + secondary)

12. NÚMEROS OEIS
    - Incluir números A- para todas as sequências discutidas
    - Listar em ordem crescente ao final do artigo

13. PONTUAÇÃO E GRAMÁTICA
    - American spelling
    - Voz ativa (evitar passiva)
    - Sem contrações (don't → do not)
    - Oxford comma
    - "i.e.," e "e.g.," com vírgula
    - "et al." sem itálico

14. ERROS COMUNS DE LaTeX
    - Todo math em math mode ($...$)
    - Pontuação de sentenças DENTRO de displayed equations, FORA de inline math
    - \mathbb para blackboard bold (Z, Q, R, C)
    - \gcd, \min, \max — usar comandos nativos
    - \mid para "divide" (a | b → a \mid b)
    - \binom ou \choose para coeficientes binomiais
    - \text para palavras em math mode
    - \ldots (entre vírgulas) e \cdots (entre operadores)
    - \geq e \leq (NÃO \geqslant, \leqslant)
    - \lfloor/\rfloor e \lceil/\rceil (não colchetes)
    - \bmod e \pmod para congruências
    - Aspas ``assim'' (não "assim")
    - Não usar eqnarray (usar align)
    - Não usar \\, \noindent, \newpage, \bigskip, \pagebreak, \linebreak
    - Remover caracteres não-ASCII (acentos usar LaTeX: \'e, \`a, etc.)

15. ARQUIVO FINAL
    - Verificar .log e .blg por erros
    - Submeter em PDF

============================================================
LINKS ÚTEIS
============================================================
- Style guide completo:
  https://cs.uwaterloo.ca/journals/JIS/format.html
- Arquivo .bst para BibTeX:
  https://cs.uwaterloo.ca/journals/JIS/jis.bst
- Abreviações MR:
  http://www.ams.org/msnhtml/serials.pdf
- Classificações AMS 2020:
  https://mathscinet.ams.org/msnhtml/msc2020.pdf
- Enviar sequências novas: https://oeis.org
- Detector de caracteres não-ASCII:
  https://pages.cs.wisc.edu/~markm/ascii.html
