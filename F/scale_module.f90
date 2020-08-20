module scale_module

    use mpi

    implicit none

    ! note: need to have at least two variables in order for a GEP instruction to appear in the LLVM IR.
    ! the first can be accesses using the handler of the struct, but the subsequent ones need an offset/address
    integer :: comm_size, comm_rank, fake_scale_variable

end module scale_module
