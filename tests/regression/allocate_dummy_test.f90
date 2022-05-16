! RUN: flang -c -O2 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load %libdir/libLLVMOverflowToolPass.so -oft-debug -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll 
! RUN: %bindir/check_marked_lines %t1.passout.ll 21 22
! XFAIL: *

program use_module
    implicit none
    external oft_mark
    integer dummy1, dummy2, dummy3
    integer, dimension (:), allocatable :: array
    
    allocate(array(4))
    !call oft_mark(dummy1)
    call oft_mark(dummy2)
    call oft_mark(dummy3)
    !array(1) = dummy1
    array(2) = dummy2
    array(3) = dummy3

    !write(*,*) "Result 1: ", 3 * array(1) !mul should be marked
    write(*,*) "Result 1: ", 5 * array(2) !mul should be marked
    write(*,*) "Result 2: ", 7 * array(3) !mul should be marked
end program use_module
!! this pattern can't be traced at the moment.
!! You trace the dummy but miss it being save to the array. 
