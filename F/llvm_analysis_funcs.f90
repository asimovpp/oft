module llvm_analysis_funcs
    implicit none
    private
    public :: print_val
    contains
        subroutine print_val(v)
            implicit none
            integer, intent(in) :: v
            print*, 'Value is ', v
        end subroutine print_val

end module llvm_analysis_funcs
