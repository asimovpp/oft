// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12 13

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int size, rank, intermediate;
    oft_mark_(&size);
    oft_mark_(&rank);
    intermediate = size + rank; //add should be marked
    printf("Number is %d\n", intermediate * 7); //mul should be marked
    return 0;
}
