// RUN: mpicc -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: grep ".*given to.*Line 14.*" %t1.passout.ll

#include <mpi.h>
#include <stdio.h>

int main() {
    MPI_Init(NULL, NULL);
    int size;
    int indirect_overflow = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for (int i = 0; i < size; i++)
        indirect_overflow += 7 + i; //multiplication should be marked as a potential scale overflow
    printf("Number is %d\n", indirect_overflow);
    MPI_Finalize();
    return 0;
}
