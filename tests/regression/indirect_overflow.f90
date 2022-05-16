! RUN: flang -c -O1 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -loop-simplify -S -o %t1.ll %t1.ll 
! RUN: opt -load %libdir/libLLVMOverflowToolPass.so -oft-trace-loops -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa,scev-aa,globals-aa,cfl-anders-aa,cfl-steens-aa,scoped-noalias-aa,type-based-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 13 14

program indirect_overflow_test
    implicit none
    external oft_mark
    integer i, rank, val
    call oft_mark(rank)
    val = 1
    do i = 1, rank ! add should be marked
        val = (val * 3) ! add should be marked
    end do
    print*, 'Number is', val
end

! this test is difficult to pass in an obvious/automatic way, because
! a number of extra operations are marked as occuring on the "end do" line, but -O1 works
