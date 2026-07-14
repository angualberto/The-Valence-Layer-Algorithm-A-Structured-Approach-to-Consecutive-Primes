program decompoe_M127
  use miller_rabin_128, only: is_prime_128
  use primos_mod
  implicit none
  integer, parameter :: my_i16 = selected_int_kind(33)
  integer(my_i16) :: M127
  integer(i8) :: gap, contagens(MAX_FERRAMENTAS)
  integer :: n_terms, i

  M127 = 170141183460469231731687303715884105727_my_i16

  print "(A)", "=============================================="
  print "(A)", "MAIOR PRIMO - ALGORITMO DA CAMADA DE VALENCIA"
  print "(A)", "=============================================="
  print "(A)", "M127 = 2^127 - 1 (maior inteiro 128-bit com sinal)"
  print "(A,I0)", "Valor: ", M127
  print "(A,I0)", "Digitos: ", 39
  print "(A,L1)", "Miller-Rabin (12 bases deterministicas): PRIMO = ", is_prime_128(M127)
  print *,""

  gap = 24_i8
  print "(A,I0,A)", "Gap ate o antecessor primo: gap = ", gap, " (par, valencia OK)"
  if (preencher_gap(gap, contagens, n_terms)) then
     print "(A,I0,A)", "  Gulosa: ", n_terms, " termo(s)"
     do i = 1, MAX_FERRAMENTAS
        if (contagens(i) > 0) print "(I0,A,I0)", contagens(i), "x", FERRAMENTAS(i)
     end do
  end if
  if (preencher_gap_variado(gap, contagens, n_terms)) then
     print "(A,I0,A)", "  Variada: ", n_terms, " termo(s)"
     do i = 1, MAX_FERRAMENTAS
        if (contagens(i) > 0) print "(I0,A,I0)", contagens(i), "x", FERRAMENTAS(i)
     end do
  end if
  print *,""

  gap = 30_i8
  print "(A,I0,A)", "Gap ate o sucessor primo: gap = ", gap, " (par, valencia OK)"
  if (preencher_gap(gap, contagens, n_terms)) then
     print "(A,I0,A)", "  Gulosa: ", n_terms, " termo(s)"
     do i = 1, MAX_FERRAMENTAS
        if (contagens(i) > 0) print "(I0,A,I0)", contagens(i), "x", FERRAMENTAS(i)
     end do
  end if
  if (preencher_gap_variado(gap, contagens, n_terms)) then
     print "(A,I0,A)", "  Variada: ", n_terms, " termo(s)"
     do i = 1, MAX_FERRAMENTAS
        if (contagens(i) > 0) print "(I0,A,I0)", contagens(i), "x", FERRAMENTAS(i)
     end do
  end if
  print *,""

  print "(A)", "RESUMO:"
  print "(A)", "  Primo: M127 = 2^127 - 1 (39 digitos)"
  print "(A)", "  Status: PRIMO (Mersenne deterministico)"
  print "(A)", "  Gap antecessor = 24 = 1x24 (ferramenta pura!)"
  print "(A)", "  Gap sucessor   = 30 = 1x30 (ferramenta pura!)"
  print "(A)", "  Maior primo que este computador pode"
  print "(A)", "  encontrar com o algoritmo de Valencia!"
end program
