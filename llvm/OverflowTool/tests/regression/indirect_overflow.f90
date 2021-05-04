! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 11 12 

program indirect_overflow_test
    implicit none
    external oft_mark
    integer i, rank, val
    call oft_mark(rank)
    val = 0
    do i = 0, rank, 1 ! add should be marked
        val = val + i ! add should be marked
    end do
    print*, 'Number is', val
end
