#include <stdio.h>
#include <mpi.h>

//big_num will overflow a 32-bit integer when multiplied by 8
#define BIG_NUM 268435457

int __attribute__ ((noinline)) calc_mostly_ok(int r) {
    return r * BIG_NUM;
}

int __attribute__ ((noinline)) calc_mostly_ok_pointer(int* r) {
    return *r * BIG_NUM;
}

int main(int argc, char *argv[]) {
    MPI_Init(NULL, NULL);
    int i, rank, size, indirect_overflow, direct_overflow, mostly_ok_int, intermediate_calc, intermediate_end, pointer_calc, func_pointer_calc;
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

    int* rankp = &(rank);
    pointer_calc = *rankp * BIG_NUM;
    func_pointer_calc = calc_mostly_ok_pointer(rankp);

    printf("Rank %d; mostly_ok_int=%d, direct_overflow=%d, indirect_overflow=%d, user_input_result=%d, intermediate_end=%d, pointer_calc=%d, func_pointer_calc=%d\n", 
           rank, mostly_ok_int, direct_overflow, indirect_overflow, user_input_result, intermediate_end, pointer_calc, func_pointer_calc);

    MPI_Finalize();
    return 0;
}


//2^28
//#define BIG_NUM 268435456
//2^28 + 1
//#define BIG_NUM 268435457
