! RUN: mpifort -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: grep ".*given to.*Line 19.*" %t1.passout.ll
! RUN: grep ".*given to.*Line 20.*" %t1.passout.ll


program use_module
    
    implicit none
    include 'mpif.h'
    integer, dimension (:), allocatable :: array
    integer ierr
    call initialise_mpi()
    
    allocate(array(2))
    call MPI_Comm_rank(MPI_COMM_WORLD, array(1), ierr)    
    call MPI_Comm_rank(MPI_COMM_WORLD, array(2), ierr)    

    write(*,*) "Result 1: ", 3 * array(1) !multiplication should be marked as a potential scale overflow
    write(*,*) "Result 2: ", 7 * array(1) !multiplication should be marked as a potential scale overflow

    call finalise_mpi()

end program use_module
