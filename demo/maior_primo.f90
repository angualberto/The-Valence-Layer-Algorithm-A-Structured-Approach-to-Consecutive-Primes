program maior_primo
  use primos_mod
  implicit none
  integer(i8) :: n, max_n
  max_n = 9223372036854775807_i8  ! 2^63 - 1
  n = max_n
  do
     if (is_prime(n)) then
        print "(A,I0)", "Maior primo < 2^63: ", n
        print "(A,I0)", "Diferenca de 2^63-1: ", max_n - n
        exit
     end if
     n = n - 2_i8  ! impares apenas
     if (mod(n, 1000000_i8) == 1) print "(A,I0,A)", "Testando ", n, "..."
  end do
end program
