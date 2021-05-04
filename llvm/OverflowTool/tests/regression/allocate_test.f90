! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 15 16 


program use_module
    implicit none
    external oft_mark
    integer, dimension (:), allocatable :: array
    
    allocate(array(2))
    call oft_mark(array(1))
    call oft_mark(array(2))

    write(*,*) "Result 1: ", 3 * array(1) !mul should be marked
    write(*,*) "Result 2: ", 7 * array(2) !mul should be marked
end program use_module
