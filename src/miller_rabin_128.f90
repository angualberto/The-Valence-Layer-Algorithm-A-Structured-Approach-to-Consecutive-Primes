module miller_rabin_128
  implicit none
  integer, parameter :: i16 = selected_int_kind(33)
  integer(i16), parameter :: BASES_128(12) = [2_i16, 3_i16, 5_i16, 7_i16, 11_i16, 13_i16, &
       17_i16, 19_i16, 23_i16, 29_i16, 31_i16, 37_i16]

contains

  function add_mod(a, b, m) result(res)
    integer(i16), intent(in) :: a, b, m
    integer(i16) :: res
    if (a > m - b) then
       res = a - (m - b)
    else
       res = a + b
    end if
  end function add_mod

  function mul_mod(a, b, m) result(res)
    integer(i16), intent(in) :: a, b, m
    integer(i16) :: res, aa, bb
    res = 0_i16
    aa = a
    bb = mod(b, m)
    do while (bb > 0_i16)
       if (mod(bb, 2_i16) == 1_i16) res = add_mod(res, aa, m)
       aa = add_mod(aa, aa, m)
       bb = bb / 2_i16
    end do
  end function mul_mod

  function pow_mod(a, b, m) result(res)
    integer(i16), intent(in) :: a, b, m
    integer(i16) :: res, base, expo
    res = 1_i16
    base = mod(a, m)
    expo = b
    do while (expo > 0_i16)
       if (mod(expo, 2_i16) == 1_i16) res = mul_mod(res, base, m)
       base = mul_mod(base, base, m)
       expo = expo / 2_i16
    end do
  end function pow_mod

  function is_prime_128(n) result(res)
    integer(i16), intent(in) :: n
    logical :: res
    integer(i16) :: d, a, x
    integer :: s, i, r

    if (n < 2_i16) then
       res = .false.; return
    end if
    if (n == 2_i16 .or. n == 3_i16) then
       res = .true.; return
    end if
    if (mod(n, 2_i16) == 0_i16) then
       res = .false.; return
    end if
    if (n < 9_i16) then
       res = .true.; return
    end if
    if (mod(n, 3_i16) == 0_i16) then
       res = .false.; return
    end if

    d = n - 1_i16
    s = 0
    do while (mod(d, 2_i16) == 0_i16)
       d = d / 2_i16
       s = s + 1
    end do

    do i = 1, 12
       a = BASES_128(i)
       if (a >= n) cycle
       x = pow_mod(a, d, n)
       if (x == 1_i16 .or. x == n - 1_i16) cycle
       do r = 1, s - 1
          x = mul_mod(x, x, n)
          if (x == n - 1_i16) exit
       end do
       if (x /= n - 1_i16) then
          res = .false.; return
       end if
    end do
    res = .true.
  end function is_prime_128

end module miller_rabin_128
