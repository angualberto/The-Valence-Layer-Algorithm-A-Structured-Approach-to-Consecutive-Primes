program test_args
  implicit none
  integer :: n
  character(len=32) :: arg
  print *, "nargs:", command_argument_count()
  if (command_argument_count() >= 1) then
    call get_command_argument(1, arg)
    print *, "arg1:", trim(arg)
    read(arg, *) n
    print *, "n =", n
  end if
end program