! RUN: flang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
! RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2>%t1.passout.ll
! RUN: %bindir/check_marked_lines %t1.passout.ll 25 26
! XFAIL: *

module scale_module
    implicit none
    external oft_mark
    ! note: need to have at least two variables in order for a GEP instruction to appear in the LLVM IR.
    ! the first can be accesses using the handler of the struct, but the subsequent ones need an offset/address
    integer :: size, rank, fake_var

contains
    subroutine mod_init()
        call oft_mark(size)
        call oft_mark(rank)
        fake_var = 42
    end subroutine mod_init
end module scale_module

program annotation_test
    use scale_module
    implicit none
    call mod_init() ! not really needed since oft_mark'd variables will be traced anyway
    print*, 'Number is', size * 3 !mul should be marked
    print*, 'Number is', rank * 7 !mul should be marked
    print*, 'Number is', fake_var * 11
end

! test test would work if the first variable in the module is not traced.
! if the first variable is traced, the whole module gets marked.
! this is because the IR is "lazy" and doesn't do an all-zero gep for the first element.
