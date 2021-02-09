program hello
    implicit none
    external oft_mark
    integer a_number

    a_number = 42 
    call oft_mark(a_number)
    
    print*, 'The number', a_number * 3
end
