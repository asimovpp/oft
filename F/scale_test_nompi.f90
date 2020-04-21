program hello
    implicit none
    !big_num will overflow a 32-bit integer when multiplied by 8
    !integer rank, size, ierror, tag, status(MPI_STATUS_SIZE)
    integer, parameter :: BIG_NUM = 268435456
    integer i, rank, size, indirect_overflow, direct_overflow, mostly_ok_int

    size = 8
    rank = 2
    
    print*, 'node', rank, ': hello world'
    
    indirect_overflow = 0
    do i = 0, size, 1
        indirect_overflow = indirect_overflow + BIG_NUM
    end do

    direct_overflow = size * BIG_NUM
    mostly_ok_int = rank * BIG_NUM

    print*, 'Rank', rank, 'mostly_ok_int=', mostly_ok_int, 'direct_overflow=', direct_overflow, 'indirect_overflow=', indirect_overflow

end
