program use_module

    use scale_module
    use set_module

    implicit none

    call initialise_mpi()

    call scale_init()

    write(*,*) "MPI comm size is ", comm_size
    
    call finalise_mpi()

end program use_module
