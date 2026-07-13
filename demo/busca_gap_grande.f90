program busca_gap_grande
  use primos_mod
  implicit none
  integer(i8) :: prime, next_prime, max_gap, gap
  integer(i8) :: contagens(MAX_FERRAMENTAS), best_contagens(MAX_FERRAMENTAS)
  integer(i8) :: start_val, end_val
  integer :: n_terms, best_terms, n_primos, i
  character(len=32) :: arg

  start_val = 1000000000000_i8
  end_val = 1000010000000_i8

  if (command_argument_count() >= 1) then
     call get_command_argument(1, arg); read(arg, *) start_val
  end if
  if (command_argument_count() >= 2) then
     call get_command_argument(2, arg); read(arg, *) end_val
  end if

  prime = start_val
  if (.not. is_prime(prime)) then
     do
        prime = prime + 1_i8
        if (is_prime(prime)) exit
     end do
  end if

  max_gap = 0
  best_terms = 0
  n_primos = 0

  write(*, '(A)') '# Busca por gaps grandes entre primos consecutivos'
  write(*, '(A,I0,A,I0)') '# Range: ', start_val, ' ate ', end_val

  do
     call proximo_primo(prime, next_prime, gap, contagens, n_terms)
     n_primos = n_primos + 1

     if (gap > max_gap) then
        max_gap = gap
        best_contagens = contagens
        best_terms = n_terms
        write(*, '(A,I0,A,I0,A,I0,A,I0)') '>>> Novo recorde! Gap: ', gap, &
             '  em ', prime, ' -> ', next_prime, '  Termos: ', n_terms
        write(*, '(A, *(I0, A, I0, A))') '    Ferramentas: ', &
             (contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)
     end if

     prime = next_prime
     if (prime >= end_val) exit
     if (mod(n_primos, 10000) == 0) write(*, '(A,I0,A,I0)') '# ', n_primos, ' primos, ultimo gap: ', gap
  end do

  write(*, '(A)') ''
  write(*, '(A)') '# === RESULTADO FINAL ==='
  write(*, '(A,I0)') '# Total de primos no range: ', n_primos
  write(*, '(A,I0)') '# Gap maximo encontrado: ', max_gap
  write(*, '(A,I0)') '# Termos na decomposicao: ', best_terms
  write(*, '(A, *(I0, A, I0, A))') 'Ferramentas: ', &
       (best_contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)

end program busca_gap_grande
