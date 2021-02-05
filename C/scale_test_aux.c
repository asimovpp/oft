#include "scale_test.h"
#include <mpi.h>
#include <stdio.h>

int __attribute__ ((noinline)) calc_mostly_ok(int r) {
    return r * BIG_NUM;
}

int __attribute__ ((noinline)) calc_mostly_ok_pointer(int* r) {
    return *r * BIG_NUM;
}

int __attribute__ ((noinline)) calc_but_dont_return_scale(int r) {
    printf("calc_but_dont_return result: %d\n", r * BIG_NUM);
    return 62;
}

int global_scale_var;
void __attribute__ ((noinline)) set_global_scale_var() {
//    printf("global var before setting is %d\n", global_scale_var);
    MPI_Comm_size(MPI_COMM_WORLD, &global_scale_var);
//    printf("global var after setting is %d\n", global_scale_var);
}

void __attribute__ ((noinline)) set_struct_scale_var(struct scale_vars* sv) {
    MPI_Comm_size(MPI_COMM_WORLD, &(sv->size));
    MPI_Comm_rank(MPI_COMM_WORLD, &(sv->rank));
}
