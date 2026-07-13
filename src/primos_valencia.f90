program primos_valencia
  use primos_mod
  implicit none
  integer :: i, count, n_terms, best_terms, n_primos, n_terms_v
  integer(i8) :: start_number, limit_number, prime, next_prime, gap, max_gap, best_prime
  integer(i8) :: contagens(MAX_FERRAMENTAS), best_contagens(MAX_FERRAMENTAS)
  integer(i8) :: contagens_v(MAX_FERRAMENTAS)
  character(len=256) :: filename
  logical :: print_screen, busca_max, tem_ate, tem_quantos, variado
  character(len=32) :: arg

  start_number = 2_i8
  count = 10
  limit_number = 0_i8
  filename = 'primos.txt'
  print_screen = .true.
  busca_max = .false.
  variado = .false.
  tem_ate = .false.; tem_quantos = .false.

  i = 1
  do while (i <= command_argument_count())
     call get_command_argument(i, arg)
     select case (trim(arg))
     case ('-inicio')
        i = i + 1
        call get_command_argument(i, arg)
        read(arg, *) start_number
     case ('-quantos')
        i = i + 1
        call get_command_argument(i, arg)
        read(arg, *) count; tem_quantos = .true.
     case ('-ate')
        i = i + 1
        call get_command_argument(i, arg)
        read(arg, *) limit_number; tem_ate = .true.
     case ('-arquivo')
        i = i + 1
        call get_command_argument(i, arg)
        filename = trim(arg)
     case ('-silencioso')
        print_screen = .false.
     case ('-maxgap')
        busca_max = .true.
     case ('-variado')
        variado = .true.
     case default
        write(*,*) 'Argumento desconhecido: ', trim(arg)
        write(*,*) 'Uso: -inicio X -quantos N -ate LIMITE -arquivo NOME -silencioso -maxgap -variado'
        stop
     end select
     i = i + 1
  end do

  if (busca_max) then
     if (.not. (tem_ate .or. tem_quantos)) then
        write(*,*) 'Erro: -maxgap requer -ate ou -quantos'
        stop
     end if

     prime = start_number
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
           if (print_screen) write(*, '(A,I0,A,I0)') '>>> Gap: ', gap, '  em ', prime
        end if
        prime = next_prime
        if (tem_quantos .and. n_primos >= count) exit
        if (tem_ate .and. prime >= limit_number) exit
     end do

     write(*, '(A)') ''
     write(*, '(A,I0)') 'Primos: ', n_primos
     write(*, '(A,I0)') 'Gap maximo: ', max_gap
     write(*, '(A,I0,A,I0)') '  em ', best_prime, ' -> ', best_prime + max_gap

     ! Decomposicao gulosa (padrao)
     write(*, '(A,I0)') 'Termos (guloso): ', best_terms
     write(*, '(A,*(I0,A,I0,A))') '  ', &
          (best_contagens(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)

     if (variado) then
        ! Decomposicao variada (maximo de ferramentas diferentes)
        if (preencher_gap_variado(max_gap, contagens_v, n_terms_v)) then
           write(*, '(A,I0)') 'Termos (variado): ', n_terms_v
           write(*, '(A,*(I0,A,I0,A))') '  ', &
                (contagens_v(i), 'x', FERRAMENTAS(i), ' ', i=1, MAX_FERRAMENTAS)
        end if
     end if
  else
     if (.not. (tem_ate .or. tem_quantos)) then
        write(*,*) 'Erro: deve especificar -quantos ou -ate'
        stop
     end if
     call gerar_lista_primos(start_number, count, limit_number, filename, print_screen)
  end if

end program primos_valencia
