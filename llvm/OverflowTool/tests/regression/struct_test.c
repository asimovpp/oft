// RUN: mpicc -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: grep ".*given to.*Line 27.*" %t1.passout.ll
// RUN: grep ".*given to.*Line 28.*" %t1.passout.ll

// OLD: x=$( grep "given to" %t1.passout.ll | wc -l ); [ $x -eq 2 ]

#include <mpi.h>
#include <stdio.h>

struct scale_vars {
    int rank;
    int size;
};

void __attribute__((noinline)) set_struct_scale_var(struct scale_vars *sv) {
    MPI_Comm_size(MPI_COMM_WORLD, &(sv->size));
    MPI_Comm_rank(MPI_COMM_WORLD, &(sv->rank));
}

int main() {
    MPI_Init(NULL, NULL);
    int struct_res1, struct_res2;
    struct scale_vars sv;

    set_struct_scale_var(&sv);
    struct_res1 = sv.rank * 3; //multiplication should be marked as a potential scale overflow
    struct_res2 = sv.size * 7; //multiplication should be marked as a potential scale overflow

    printf("struct_res1=%d, struct_res2=%d\n", struct_res1, struct_res2);

    MPI_Finalize();
    return 0;
}
