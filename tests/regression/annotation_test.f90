! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 10 

program annotation_test
    implicit none
    external oft_mark
    integer val
    call oft_mark(val)
    print*, 'Number is', val * 3 !mul should be marked
end
