program agle_distribution
    use primos_mod
    implicit none
    real(8) :: x, r, f_real, f_quant, epsilon
    real(4) :: f32
    integer(4) :: out_bytes
    integer(8) :: raw_bits
    integer(8) :: contagens(MAX_FERRAMENTAS)
    integer(i8) :: prime, next_prime, real_gap
    integer(i8) :: agle_gap_raw, agle_candidate, candidate_prime
    integer :: i, j, n_primes, n_agle_per_gap, n_terms
    integer, parameter :: NBINS = 100
    integer :: hist_real(0:NBINS-1), hist_agle_any(0:NBINS-1)
    integer :: total_agle_primes, agle_tries
    integer(i8) :: sum_real_gap
    real(8) :: mean_real_gap, mean_agle_gap
    character(len=32) :: arg

    r = 3.99999999999999_8
    x = 0.12345678901234_8
    n_primes = 5000
    n_agle_per_gap = 100

    if (command_argument_count() >= 1) then
        call get_command_argument(1, arg)
        read(arg, *) n_primes
    end if
    if (command_argument_count() >= 2) then
        call get_command_argument(2, arg)
        read(arg, *) n_agle_per_gap
    end if

    prime = 1000000000000_i8
    if (.not. is_prime(prime)) then
        do
            prime = prime + 1_i8
            if (is_prime(prime)) exit
        end do
    end if

    hist_real = 0; hist_agle_any = 0
    sum_real_gap = 0; total_agle_primes = 0; agle_tries = 0
    mean_agle_gap = 0.0_8

    do i = 1, n_primes
        call proximo_primo(prime, next_prime, real_gap, contagens, n_terms)

        if (real_gap / 2 < NBINS) hist_real(real_gap / 2) = hist_real(real_gap / 2) + 1
        sum_real_gap = sum_real_gap + real_gap

        ! AGLE: generate n_agle_per_gap gaps, check if each lands on ANY prime
        do j = 1, n_agle_per_gap
            call agle_byte(out_bytes)
            agle_gap_raw = int(abs(out_bytes), i8) * 2_i8 + 2_i8
            agle_tries = agle_tries + 1
            mean_agle_gap = mean_agle_gap + real(agle_gap_raw, 8)

            candidate_prime = prime + agle_gap_raw
            if (candidate_prime > next_prime .and. is_prime(candidate_prime)) then
                total_agle_primes = total_agle_primes + 1
                if (agle_gap_raw / 2 < NBINS) then
                    hist_agle_any(agle_gap_raw / 2) = hist_agle_any(agle_gap_raw / 2) + 1
                end if
            end if
        end do

        prime = next_prime
        if (mod(i, 1000) == 0) then
            write(*, '(A,I0,A,F5.1,A)') '# ', i, ' primos, densidade AGLE: ', &
                100.0 * total_agle_primes / max(agle_tries, 1), '%'
        end if
    end do

    mean_real_gap = real(sum_real_gap, 8) / real(n_primes, 8)
    mean_agle_gap = mean_agle_gap / real(max(agle_tries, 1), 8)

    write(*, '(A)') '# === SUMARIO ==='
    write(*, '(A,F10.4)') '# Media gaps reais: ', mean_real_gap
    write(*, '(A,F10.4)') '# Media gaps AGLE (brutos): ', mean_agle_gap
    write(*, '(A,I0,A,I0)') '# Total AGLE hits (qualquer primo apos): ', total_agle_primes, ' de ', agle_tries
    write(*, '(A,F6.2,A)') '# Densidade de primos via AGLE: ', &
        100.0 * total_agle_primes / max(agle_tries, 1), '%'
    write(*, '(A,F6.2,A)') '# Densidade natural (1/ln(p)): ', 100.0 / log(real(prime, 8)), '%'
    write(*, '(A)') ''
    write(*, '(A)') '# Histograma: gap  freq_real  freq_agle_any'
    do i = 0, NBINS - 1
        if (hist_real(i) > 0 .or. hist_agle_any(i) > 0) then
            write(*, '(I4, 2(1X,I8))') i * 2, hist_real(i), hist_agle_any(i)
        end if
    end do

contains

    subroutine agle_byte(byte_out)
        integer(4), intent(out) :: byte_out
        f_real = r * x * (1.0_8 - x)
        f32 = real(f_real, 4)
        f_quant = real(f32, 8)
        epsilon = f_real - f_quant
        x = abs(epsilon * 1.0d10)
        x = x - int(x)

        raw_bits = transfer(epsilon, raw_bits)
        byte_out = ieor(int(raw_bits, 4), int(ishft(raw_bits, -32), 4))
        byte_out = ieor(byte_out, ishft(byte_out, 13))
        byte_out = ieor(byte_out, ishft(byte_out, -17))
        byte_out = ieor(byte_out, ishft(byte_out, 5))
        byte_out = int(mod(int(z'0019660d',8) * int(byte_out,8) + int(z'3c6ef35f',8), int(z'100000000',8)),4)
        byte_out = ieor(byte_out, ishft(byte_out, 7))
        byte_out = ieor(byte_out, ishft(byte_out, -9))
        byte_out = ieor(byte_out, ishft(byte_out, 13))
    end subroutine agle_byte

end program agle_distribution
