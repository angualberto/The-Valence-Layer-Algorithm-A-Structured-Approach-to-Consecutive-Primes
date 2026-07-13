program test_mr128
  use miller_rabin_128
  implicit none
  integer(i16) :: n

  n = 2_i16
  print "(A,I0,A,L1)", "is_prime(", n, ") = ", is_prime_128(n)

  n = 9223372036854775783_i16
  print "(A,I0,A,L1)", "is_prime(", n, ") = ", is_prime_128(n)

  n = 9223372036854775807_i16
  print "(A,I0,A,L1)", "is_prime(", n, ") = ", is_prime_128(n)

  n = 170141183460469231731687303715884105727_i16
  print "(A)", "Testando 2^127 - 1 (Mersenne primo conhecido)..."
  print "(A,L1)", "is_prime(2^127-1) = ", is_prime_128(n)

  n = 340282366920938463463374607431768211297_i16
  print "(A)", "Testando primo grande 128-bit..."
  print "(A,L1)", "is_prime = ", is_prime_128(n)

  n = 340282366920938463463374607431768211455_i16
  print "(A,L1)", "is_prime(composto) = ", is_prime_128(n)
end program
