program busca_gap_rapido
  use primos_mod
  implicit none
  integer(i8) :: prime, next_prime, max_gap, gap
  integer(i8) :: contagens(MAX_FERRAMENTAS), best_contagens(MAX_FERRAMENTAS)
  integer(i8) :: start_val, end_val, best_prime
  integer :: n_terms, best_terms, n_primos, i
  character(len=32) :: arg

  start_val = 1000000000000000_i8
  end_val = 1000000010000000_i8

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

  max_gap = 0; best_terms = 0; n_primos = 0; best_prime = 0

  do
     call proximo_primo(prime, next_prime, gap, contagens, n_terms)
     n_primos = n_primos + 1
     if (gap > max_gap) then
        max_gap = gap; best_contagens = contagens; best_terms = n_terms; best_prime = prime
     end if
     prime = next_prime
     if (prime >= end_val) exit
  end do

  write(*, '(A,I0,A,I0,A)') 'Range [', start_val, ', ', end_val, ']'
  write(*, '(A,I0)') 'Primos: ', n_primos
  write(*, '(A,I0)') 'Gap maximo: ', max_gap
  write(*, '(A,I0,A,I0)') '  em ', best_prime, ' -> ', best_prime + max_gap
  write(*, '(A,I0,A)') 'Termos: ', best_terms
  write(*, '(A,*(I0,A,I0,A))') '  ', &
       (best_contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)

end program busca_gap_rapido
