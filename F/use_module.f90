program use_module

    use scale_module
    use set_module

    implicit none
    integer :: scale_array(*) 
    integer, dimension (:), allocatable :: allocatable_scale_array
    !TODO: integer :: normal_array(64) size known statically
    !TODO: integer :: normal_array(n)  automatic array. Probably needs to be in a routine. 
    external op_malloc
    integer*8 :: op_malloc
    pointer(ptr_arr, scale_array)
    ptr_arr = op_malloc(2*4)

    call initialise_mpi()
    
    call MPI_Comm_rank(MPI_COMM_WORLD, scale_array(1), ierr)    
    call MPI_Comm_size(MPI_COMM_WORLD, scale_array(2), ierr)

    call scale_init()
    
    allocate(allocatable_scale_array(comm_size+1))
    call MPI_Comm_rank(MPI_COMM_WORLD, allocatable_scale_array(1), ierr)    
    call MPI_Comm_rank(MPI_COMM_WORLD, allocatable_scale_array(2), ierr)    

    write(*,*) "Hello from 3x rank ", 3 * comm_rank, " of ", 3 * comm_size, " 3x total ranks."
    write(*,*) "This one should not be traced ", 3 * fake_scale_variable

    write(*,*) "Here are the 3 global DATA variables: ", 3 * global_1, 3 * global_2, 3 * global_3 
    write(*,*) "Here are the 2 common global variables: ", 3 * common_global_1, 3 * common_global_2
    
    write(*,*) "Here are the 2 array variables: ", 3 * scale_array(1), 3 * scale_array(2)
    write(*,*) "Here are the 2 allocatable array variables ", 7 * allocatable_scale_array(1), 7 * allocatable_scale_array(2)
    

    call finalise_mpi()

end program use_module
