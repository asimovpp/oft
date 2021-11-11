! RUN: flang -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load %libdir/libLLVMOverflowToolPass.so -oft-debug -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 16 17 18


program use_module
    implicit none
    external oft_mark
    integer, dimension (:), allocatable :: array
    
    allocate(array(4))
    call oft_mark(array(1))
    call oft_mark(array(2))
    call oft_mark(array(3))

    write(*,*) "bla", 3 * array(1) !mul should be marked
    write(*,*) "bla", 5 * array(2) !mul should be marked
    write(*,*) "bla", 7 * array(3) !mul should be marked
    write(*,*) "bla", 11 * array(4)
end program use_module
