// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12 13

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int rank_array[4];
    oft_mark_(&(rank_array[0]));
    oft_mark_(&(rank_array[1]));
    printf("Number is %d\n", rank_array[0] * 3); //mul should be marked
    printf("Number is %d\n", rank_array[1] * 7); //mul should be marked
    return 0;
}
