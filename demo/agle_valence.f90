program agle_valence_integration
    use primos_mod
    implicit none
    real(8) :: x, r, f_real, f_quant, epsilon
    real(4) :: f32
    integer(4) :: out_bytes, gap_aleatorio
    integer(8) :: raw_bits
    integer(8) :: contagens(MAX_FERRAMENTAS)
    integer :: i, n_tests, n_terms, hits, attempts
    integer(i8) :: start_prime, next_prime, real_gap, agle_gap
    integer(i8) :: agle_candidate, real_next_prime

    r = 3.99999999999999_8
    x = 0.12345678901234_8

    start_prime = 1000000000000_i8
    ! Avança para o primeiro primo real
    if (.not. is_prime(start_prime)) then
        do
            start_prime = start_prime + 1_i8
            if (is_prime(start_prime)) exit
        end do
    end if

    n_tests = 1000
    hits = 0

    write(*, '(A)') '# AGLE + Algoritmo da Camada de Valencia'
    write(*, '(A, I0)') '# Inicio: ', start_prime
    write(*, '(A, I0)') '# Testes: ', n_tests
    write(*, '(A)') '# Primo_Base  AGLE_Candidate  AGLE_Gap  Real_Next  Real_Gap  Acertou'

    do i = 1, n_tests
        ! Encontra o proximo primo REAL
        call proximo_primo(start_prime, real_next_prime, real_gap, contagens, n_terms)

        ! Tenta encontrar primo com gaps gerados pelo AGLE
        attempts = 0
        agle_candidate = 0_i8
        do
            call agle_byte(out_bytes)
            gap_aleatorio = abs(out_bytes) * 2 + 2
            agle_gap = int(gap_aleatorio, i8)
            attempts = attempts + 1
            if (is_prime(start_prime + agle_gap)) then
                agle_candidate = start_prime + agle_gap
                exit
            end if
            if (attempts > 10000) exit
        end do

        if (agle_candidate == real_next_prime) then
            hits = hits + 1
        end if
        write(*, '(I15, 1X, I15, 1X, I8, 1X, I15, 1X, I8, 1X, L1)') &
            start_prime, agle_candidate, agle_gap, real_next_prime, real_gap, &
            (agle_candidate == real_next_prime)

        start_prime = real_next_prime
    end do

    write(*, '(A, I0, A, I0)') 'Acertos: ', hits, ' de ', n_tests

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

end program agle_valence_integration
