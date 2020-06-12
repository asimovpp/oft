#include "scale_test.h"
#include <mpi.h>
#include <stdio.h>

int __attribute__ ((noinline)) calc_mostly_ok(int r) {
    return r * BIG_NUM;
}

int __attribute__ ((noinline)) calc_mostly_ok_pointer(int* r) {
    return *r * BIG_NUM;
}

int global_scale_var;
void __attribute__ ((noinline)) set_global_scale_var() {
//    printf("global var before setting is %d\n", global_scale_var);
    MPI_Comm_size(MPI_COMM_WORLD, &global_scale_var);
//    printf("global var after setting is %d\n", global_scale_var);
}