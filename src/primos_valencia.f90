program primos_valencia
  use primos_mod
  implicit none
  integer :: i, count
  integer(i8) :: start_number, limit_number
  character(len=256) :: filename
  logical :: print_screen
  character(len=32) :: arg

  start_number = 2_i8
  count = 10
  limit_number = 0_i8
  filename = 'primos.txt'
  print_screen = .true.

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
        read(arg, *) count
     case ('-ate')
        i = i + 1
        call get_command_argument(i, arg)
        read(arg, *) limit_number
     case ('-arquivo')
        i = i + 1
        call get_command_argument(i, arg)
        filename = trim(arg)
     case ('-silencioso')
        print_screen = .false.
     case default
        write(*,*) 'Argumento desconhecido: ', trim(arg)
        write(*,*) 'Uso: programa -inicio X -quantos N -ate LIMITE -arquivo NOME -silencioso'
        stop
     end select
     i = i + 1
  end do

  if (count <= 0 .and. limit_number <= 0) then
     write(*,*) 'Erro: deve especificar -quantos ou -ate'
     stop
  end if

  call gerar_lista_primos(start_number, count, limit_number, filename, print_screen)

end program primos_valencia
