! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 24 25 

module scale_module
    implicit none
    external oft_mark
    integer :: global_1, global_2, fake_var
    data global_1 /1/
    data global_2 /2/

contains
    subroutine mod_init()
        call oft_mark(global_1)
        call oft_mark(global_2)
        fake_var = 42
    end subroutine mod_init
end module scale_module

program annotation_test
    use scale_module
    implicit none
    call mod_init() ! not really needed since oft_mark'd variables will be traced anyway
    print*, 'Number is', global_1 * 3 !mul should be marked
    print*, 'Number is', global_2 * 7 !mul should be marked
    print*, 'Number is', fake_var * 11
end
