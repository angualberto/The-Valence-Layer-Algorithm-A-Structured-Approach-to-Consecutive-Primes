program maior_primo_128
  use miller_rabin_128
  implicit none
  integer(i16) :: n, max_n, passo
  integer :: n_passos
  max_n = 170141183460469231731687303715884105727_i16  ! 2^127 - 1
  n = max_n
  n_passos = 0

  ! Testa 2^127-1 primeiro (e Mersenne conhecido)
  if (is_prime_128(n)) then
     print "(A)", "2^127-1 = 170141183460469231731687303715884105727"
     print "(A)", "E primo! (Mersenne M127, conhecido)"
     print "(A,I0)", "Maior primo 128-bit: ", n
     print "(A,I0)", "Diferenca de 2^127-1: 0"
     stop
  end if

  ! Se nao for primo, busca para baixo
  do
     if (is_prime_128(n)) then
        print "(A,I0)", "Maior primo 128-bit: ", n
        print "(A,I0)", "Diferenca de 2^127-1: ", max_n - n
        exit
     end if
     n = n - 2_i16
     n_passos = n_passos + 1
     if (mod(n_passos, 100000) == 0) then
        print "(A,I0,A)", "Testando ate ", n, "..."
     end if
  end do
end program
