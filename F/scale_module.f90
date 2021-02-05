module scale_module

    use mpi

    implicit none

    ! note: need to have at least two variables in order for a GEP instruction to appear in the LLVM IR.
    ! the first can be accesses using the handler of the struct, but the subsequent ones need an offset/address
    integer :: global_1, global_2, global_3
    integer :: common_global_1, common_global_2
    integer :: comm_size, comm_rank, fake_scale_variable
    data global_1 /1/
    data global_2 /2/
    data global_3 /3/

    common /stuff/ common_global_1, common_global_2
    save /stuff/


end module scale_module
