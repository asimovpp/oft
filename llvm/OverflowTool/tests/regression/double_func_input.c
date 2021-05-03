// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 9 10 

#include <stdio.h>
extern void oft_mark_(void *);

void __attribute__ ((noinline)) do_calc(int r, int s) {
    r * 7; //mul should be marked
    s * 7; //mul should be marked
}

int main() {
    int size, rank;
    oft_mark_(&size);
    oft_mark_(&rank);
    do_calc(size, rank);
    return 0;
}
