module primos_mod
  implicit none
  integer, parameter :: i8 = selected_int_kind(18)
  integer, parameter :: i16 = selected_int_kind(33)
  integer, parameter :: dp = kind(1.0d0)
  integer, parameter :: MAX_FERRAMENTAS = 16
  integer(i8), parameter :: FERRAMENTAS(MAX_FERRAMENTAS) = [120_i8, 64_i8, 34_i8, 32_i8, 30_i8, 28_i8, 24_i8, 20_i8, &
       18_i8, 16_i8, 12_i8, 10_i8, 8_i8, 6_i8, 4_i8, 2_i8]

contains

  function mul_mod(a, b, m) result(res)
    integer(i8), intent(in) :: a, b, m
    integer(i8) :: res
    integer(i16) :: a16, b16, m16
    a16 = a; b16 = b; m16 = m
    res = int(mod(a16 * b16, m16), i8)
  end function mul_mod

  function pow_mod(a, b, m) result(res)
    integer(i8), intent(in) :: a, b, m
    integer(i8) :: res, base, expo
    res = 1_i8
    base = mod(a, m)
    expo = b
    do while (expo > 0)
       if (mod(expo, 2_i8) == 1) then
          res = mul_mod(res, base, m)
       end if
       base = mul_mod(base, base, m)
       expo = expo / 2_i8
    end do
  end function pow_mod

  function is_prime(n) result(res)
    integer(i8), intent(in) :: n
    logical :: res
    integer(i8) :: d, a, x
    integer :: s, i, r
    integer(i8), parameter :: bases(7) = [2_i8, 325_i8, 9375_i8, 28178_i8, &
         450775_i8, 9780504_i8, 1795265022_i8]

    if (n < 2_i8) then
       res = .false.
       return
    end if
    if (n == 2_i8 .or. n == 3_i8 .or. n == 5_i8 .or. n == 7_i8 .or. n == 11_i8 .or. &
        n == 13_i8 .or. n == 17_i8 .or. n == 19_i8 .or. n == 23_i8 .or. n == 29_i8) then
       res = .true.
       return
    end if
    if (mod(n, 2_i8) == 0 .or. mod(n, 3_i8) == 0 .or. mod(n, 5_i8) == 0 .or. &
        mod(n, 7_i8) == 0 .or. mod(n, 11_i8) == 0 .or. mod(n, 13_i8) == 0 .or. &
        mod(n, 17_i8) == 0 .or. mod(n, 19_i8) == 0 .or. mod(n, 23_i8) == 0 .or. &
        mod(n, 29_i8) == 0) then
       res = .false.
       return
    end if

    d = n - 1_i8
    s = 0
    do while (mod(d, 2_i8) == 0)
       d = d / 2_i8
       s = s + 1
    end do

    do i = 1, 7
       a = bases(i)
       if (a >= n) cycle
       x = pow_mod(a, d, n)
       if (x == 1_i8 .or. x == n - 1_i8) cycle
       do r = 1, s-1
          x = mul_mod(x, x, n)
          if (x == n - 1_i8) exit
       end do
       if (x /= n - 1_i8) then
          res = .false.
          return
       end if
    end do
    res = .true.
  end function is_prime

  function preencher_gap(G, contagens, n_terms) result(sucesso)
    integer(i8), intent(in) :: G
    integer(i8), intent(out) :: contagens(MAX_FERRAMENTAS)
    integer, intent(out) :: n_terms
    logical :: sucesso
    integer(i8) :: resto, qtd
    integer :: i

    if (mod(G, 2_i8) /= 0) then
       sucesso = .false.
       return
    end if

    resto = G
    contagens = 0_i8
    n_terms = 0
    do i = 1, MAX_FERRAMENTAS
       qtd = resto / FERRAMENTAS(i)
       if (qtd > 0) then
          contagens(i) = qtd
          n_terms = n_terms + int(qtd)
          resto = resto - qtd * FERRAMENTAS(i)
       end if
    end do

    if (resto == 0_i8) then
       sucesso = .true.
    else
       sucesso = .false.
    end if
  end function preencher_gap

  ! Usa cada ferramenta no maximo 1 vez (maiores primeiro), resto com 2
  function preencher_gap_variado(G, contagens, n_terms) result(sucesso)
    integer(i8), intent(in) :: G
    integer(i8), intent(out) :: contagens(MAX_FERRAMENTAS)
    integer, intent(out) :: n_terms
    logical :: sucesso
    integer(i8) :: resto
    integer :: i

    if (mod(G, 2_i8) /= 0) then
       sucesso = .false.
       return
    end if

    resto = G
    contagens = 0_i8
    n_terms = 0

    ! Tenta usar cada ferramenta (exceto 2) no maximo 1 vez
    do i = 1, MAX_FERRAMENTAS - 1
       if (resto >= FERRAMENTAS(i)) then
          contagens(i) = 1_i8
          n_terms = n_terms + 1
          resto = resto - FERRAMENTAS(i)
       end if
    end do

    ! Preenche o resto com a ferramenta 2 (ultima)
    if (mod(resto, 2_i8) == 0) then
       contagens(MAX_FERRAMENTAS) = resto / 2_i8
       n_terms = n_terms + int(resto / 2_i8)
       resto = 0_i8
    end if

    if (resto == 0_i8) then
       sucesso = .true.
    else
       sucesso = .false.
    end if
  end function preencher_gap_variado

  subroutine proximo_primo(start_number, next_prime, gap, contagens, n_terms)
    integer(i8), intent(in) :: start_number
    integer(i8), intent(out) :: next_prime, gap
    integer(i8), intent(out) :: contagens(MAX_FERRAMENTAS)
    integer, intent(out) :: n_terms
    integer(i8) :: candidato, current_gap
    logical :: sucesso

    candidato = start_number
    if (.not. is_prime(candidato)) then
       do
          candidato = candidato + 1_i8
          if (is_prime(candidato)) exit
       end do
    end if

    if (candidato == 2_i8) then
       next_prime = 3_i8; gap = 1_i8; contagens = 0_i8; n_terms = 1; return
    end if

    current_gap = 2_i8
    do
       if (is_prime(candidato + current_gap)) then
          next_prime = candidato + current_gap
          gap = current_gap
          sucesso = preencher_gap(gap, contagens, n_terms)
          if (.not. sucesso) then
             n_terms = -1; contagens = -1_i8
          end if
          return
       end if
       current_gap = current_gap + 2_i8
    end do
  end subroutine proximo_primo

  subroutine gerar_lista_primos(start, count, limit, filename, print_to_screen)
    integer(i8), intent(in) :: start, limit
    integer, intent(in) :: count
    character(len=*), intent(in) :: filename
    logical, intent(in) :: print_to_screen
    integer(i8) :: current_prime, next_p, gap
    integer(i8) :: contagens(MAX_FERRAMENTAS)
    integer :: n_terms, i, primos_gerados
    integer :: unit

    open(newunit=unit, file=filename, status='replace', action='write')

    current_prime = start
    if (.not. is_prime(current_prime)) then
       do
          current_prime = current_prime + 1_i8
          if (is_prime(current_prime)) exit
       end do
    end if

    write(unit, '(A)') '# Lista de primos - Algoritmo da Camada de Valencia'
    write(unit, '(A, I0)') '# Inicio: ', current_prime
    write(unit, '(A)') '# Ferramentas: 120, 64, 34, 20, 10, 6, 4, 2'

    primos_gerados = 0
    do
       if (print_to_screen) write(*, '(I0)') current_prime
       write(unit, '(I0)') current_prime

       primos_gerados = primos_gerados + 1

       if (count > 0 .and. primos_gerados >= count) exit
       if (limit > 0 .and. current_prime >= limit) exit

       call proximo_primo(current_prime, next_p, gap, contagens, n_terms)

       if (print_to_screen) then
          write(*, '(A, I0, A, I0)') '  Gap: ', gap, '  Termos: ', n_terms
          write(*, '(A, *(I0, A, I0, A))') '  Ferramentas: ', &
               (contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)
       end if
       write(unit, '(A, I0, A, I0)') '# Gap: ', gap, '  Termos: ', n_terms
        write(unit, '(A, *(I0, A, I0, A))') '# Ferramentas: ', &
             (contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)

       current_prime = next_p
    end do

    close(unit)
    if (print_to_screen) then
       write(*, '(A, I0)') 'Total de primos gerados: ', primos_gerados
       write(*, '(A, A)') 'Arquivo salvo em: ', filename
    end if
  end subroutine gerar_lista_primos

end module primos_mod
