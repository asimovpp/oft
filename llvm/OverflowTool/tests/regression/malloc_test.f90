! RUN: mpifort -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 21 22 

program malloc_test

    implicit none
    include 'mpif.h'
    integer :: scale_array(*) 
    integer ierr
    external op_malloc ! the definition should be in an external C file, but for the test here it is not needed
    integer*8 :: op_malloc  
    pointer(ptr_arr, scale_array)
    ptr_arr = op_malloc(2*4)

    call initialise_mpi()
    
    call MPI_Comm_rank(MPI_COMM_WORLD, scale_array(1), ierr)    
    call MPI_Comm_size(MPI_COMM_WORLD, scale_array(2), ierr)
    
    write(*,*) "Result 1: ", 3 * scale_array(1) !mul should be marked
    write(*,*) "Result 2: ", 7 * scale_array(2) !mul should be marked

    call finalise_mpi()

end program malloc_test
