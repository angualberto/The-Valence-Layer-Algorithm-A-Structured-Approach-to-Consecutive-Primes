program verifica_gap
  use primos_mod
  implicit none
  integer(i8) :: p1, p2, gap, contagens(MAX_FERRAMENTAS)
  integer :: n_terms, i
  logical :: ok

  p1 = 492113_i8
  p2 = 492227_i8
  gap = p2 - p1

  print "(A,1X,L1)", "492113 eh primo?", is_prime(p1)
  print "(A,1X,L1)", "492227 eh primo?", is_prime(p2)
  print "(A,I0,A,I0,A,I0)", "Gap: ", p1, " -> ", p2, " = ", gap

  ok = preencher_gap(gap, contagens, n_terms)
  if (ok) then
     print "(A,I0,A)", "Decomposicao em ", n_terms, " termos:"
     print "(8(I0,'x',I0,' '))", (contagens(i), FERRAMENTAS(i), i=1, MAX_FERRAMENTAS)
     print "(A,I0)", "Soma: ", sum(contagens * FERRAMENTAS)
  else
     print "(A)", "Falha na decomposicao!"
  end if
end program
