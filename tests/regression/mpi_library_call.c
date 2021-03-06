// RUN: mpicc -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12

#include <mpi.h>
#include <stdio.h>

int main() {
    MPI_Init(NULL, NULL);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Number is %d\n", size * 7); //mul should be marked
    MPI_Finalize();
    return 0;
}
