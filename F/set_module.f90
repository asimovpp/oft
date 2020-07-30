module set_module

    use scale_module

    implicit none
        
    integer :: ierr

contains
    subroutine scale_init()
        integer :: ierr

        call MPI_Comm_size(MPI_COMM_WORLD, comm_size, ierr)
    end subroutine scale_init

    subroutine initialise_mpi()
        call MPI_Init(ierr)
    end subroutine initialise_mpi
    
    subroutine finalise_mpi()
        call MPI_Finalize(ierr)
    end subroutine finalise_mpi

end module set_module
