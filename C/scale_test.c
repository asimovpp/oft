#include <stdio.h>
#include <mpi.h>
#include "scale_test.h"


//int global_scale_var;

int main(int argc, char *argv[]) {
    MPI_Init(NULL, NULL);
    int i, rank, size, indirect_overflow, direct_overflow, mostly_ok_int, intermediate_calc, intermediate_end2, intermediate_end3, func_intermediate, func_intermediate_end, intermediate_end, not_scale, not_scale_end, pointer_calc, func_pointer_calc, global_res;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Hello from rank %d\n", rank);
    printf("World size is %d\n", size);

    indirect_overflow = 0;
    for (i = 0; i < size; i++) 
        indirect_overflow += BIG_NUM + i;

    direct_overflow = size * BIG_NUM;
    mostly_ok_int = calc_mostly_ok(rank);

    int user_input_result = -1;
    if (argc == 2)
        user_input_result = ((int) argv[1]) * BIG_NUM;

    intermediate_calc = (sizeof(float) - 6) * rank;
    intermediate_end = calc_mostly_ok(intermediate_calc);
    intermediate_end2 = calc_mostly_ok(intermediate_calc + 42);
    intermediate_end3 = calc_mostly_ok(intermediate_calc + 44);

    func_intermediate = calc_mostly_ok(rank + size);
    func_intermediate_end = func_intermediate + rank;


    not_scale = calc_but_dont_return_scale(rank);
    not_scale_end = not_scale + 1;

    int* rankp = &(rank);
    pointer_calc = *rankp * BIG_NUM;
    func_pointer_calc = calc_mostly_ok_pointer(rankp);
    
//    printf("global var before setting in main is %d\n", global_scale_var);
    set_global_scale_var();
//    printf("global var after setting in main is %d\n", global_scale_var);
    global_res = global_scale_var * BIG_NUM;

    printf("Rank %d; mostly_ok_int=%d, direct_overflow=%d, indirect_overflow=%d, user_input_result=%d, intermediate_end=%d, intermediate_end2=%d, intermediate_end3=%d, func_intermediate=%d, func_intermediate_end=%d, not_scale=%d, not_scale_end=%d, pointer_calc=%d, func_pointer_calc=%d, global_res=%d\n", 
           rank, mostly_ok_int, direct_overflow, indirect_overflow, user_input_result, intermediate_end, intermediate_end2, intermediate_end3, func_intermediate, func_intermediate_end, not_scale, not_scale_end, pointer_calc, func_pointer_calc, global_res);

    MPI_Finalize();
    return 0;
}


//2^28
//#define BIG_NUM 268435456
//2^28 + 1
//#define BIG_NUM 268435457
