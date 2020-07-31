program use_module

    use scale_module
    use set_module

    implicit none

    call initialise_mpi()

    call scale_init()

    write(*,*) "Hello from 2x rank ", 2 * comm_rank, " of ", 2 * comm_size, " 2x total ranks."
    
    call finalise_mpi()

end program use_module
