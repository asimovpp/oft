! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 18 19
! XFAIL: *

program malloc_test
    implicit none
    external oft_mark
    integer :: scale_array(*) 
    external op_malloc ! the definition should be in an external C file, but for the test here it is not needed
    integer*8 :: op_malloc  
    pointer(ptr_arr, scale_array)
    ptr_arr = op_malloc(2*4)

    call oft_mark(scale_array(1))
    call oft_mark(scale_array(2))
    
    write(*,*) "Result 1: ", 3 * scale_array(1) !mul should be marked
    write(*,*) "Result 2: ", 7 * scale_array(2) !mul should be marked
end program malloc_test
! this basically works, but op_malloc_ needs to be marked as a valid bwd trace endpoint
! in some sort of tracing configuration, which hasn't been implemented yet
