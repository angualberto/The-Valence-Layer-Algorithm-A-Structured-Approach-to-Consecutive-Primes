program crivo_segmentado
  implicit none
  integer(kind=8), parameter :: SEG = 1000000_8
  integer(kind=8) :: N, low, high, i, j, p, start, total, t0, t1, rate
  integer(kind=4) :: prime_count, pc
  integer(kind=8), allocatable :: primes(:)
  logical, allocatable :: is_prime(:)

  read *, N

  call system_clock(t0, rate)

  call simple_sieve(int(sqrt(real(N,8)),8), primes, prime_count)
  total = prime_count

  low = int(sqrt(real(N,8)),8) + 1
  if (low <= 1) low = 2
  if (mod(low, 2_8) == 0) low = low + 1

  do while (low <= N)
     high = min(low + SEG - 1, N)
     allocate(is_prime(SEG))
     is_prime = .true.

     if (low <= 2_8) is_prime(1) = .false.

     do i = 1, prime_count
        p = primes(i)
        if (p * p > high) exit
        start = max(p * p, (low + p - 1) / p * p)
        if (mod(start, 2_8) == 0) start = start + p
        do j = start, high, 2_8 * p
           if (j >= low) is_prime(j - low + 1) = .false.
        end do
     end do

     pc = 0
     do j = low, high, 2_8
        if (is_prime(j - low + 1)) pc = pc + 1
     end do
     total = total + pc

     deallocate(is_prime)
     low = high + 1
     if (mod(low, 2_8) == 0) low = low + 1
  end do

  call system_clock(t1)
  print "(A,I0,A,I0,A,F6.2,A)", "Primos ate ", N, ": ", total, &
       "  Tempo: ", real(t1-t0,8)/rate, " s"
contains

  subroutine simple_sieve(N, primes, cnt)
    integer(kind=8), intent(in) :: N
    integer(kind=8), allocatable, intent(out) :: primes(:)
    integer(kind=4), intent(out) :: cnt
    logical, allocatable :: comp(:)
    integer(kind=8) :: i, j, arr_size

    if (N < 2) then; cnt = 0; allocate(primes(0)); return; end if
    arr_size = N / 2_8
    allocate(comp(arr_size))
    comp = .false.

    i = 3
    do while (i * i <= N)
       if (.not. comp(i/2_8)) then
          j = i * i
          do while (j <= N)
             comp(j/2_8) = .true.
             j = j + 2_8 * i
          end do
       end if
       i = i + 2_8
    end do

    cnt = 1
    do i = 3_8, N, 2_8
       if (.not. comp(i/2_8)) cnt = cnt + 1
    end do
    allocate(primes(cnt))
    primes(1) = 2_8
    j = 2
    do i = 3_8, N, 2_8
       if (.not. comp(i/2_8)) then; primes(j) = i; j = j + 1; end if
    end do
    deallocate(comp)
  end subroutine

end program
