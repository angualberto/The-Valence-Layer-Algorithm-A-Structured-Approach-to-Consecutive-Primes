program verifica_gap_grande
  use primos_mod
  implicit none
  integer(i8) :: p, prev, gap, contagens(MAX_FERRAMENTAS), contagens_v(MAX_FERRAMENTAS)
  integer :: n_terms, n_terms_v, i

  p = 9223372036854775783_i8

  ! Encontra o primo anterior
  prev = p - 2_i8
  do
     if (is_prime(prev)) exit
     prev = prev - 2_i8
  end do

  gap = p - prev
  print "(A,I0)", "Primo: ", p
  print "(A,I0)", "Primo anterior: ", prev
  print "(A,I0)", "Gap: ", gap

  if (preencher_gap(gap, contagens, n_terms)) then
     print "(A,I0)", "Termos (guloso): ", n_terms
     print "(A,*(I0,A,I0,A))", "  ", &
          (contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)
  end if

  if (preencher_gap_variado(gap, contagens_v, n_terms_v)) then
     print "(A,I0)", "Termos (variado): ", n_terms_v
     print "(A,*(I0,A,I0,A))", "  ", &
          (contagens_v(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)
  end if
end program
