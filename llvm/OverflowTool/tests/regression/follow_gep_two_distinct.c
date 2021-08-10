// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 15 16

#include <stdlib.h>
#include <stdio.h>
extern void oft_mark_(void *);

void __attribute__ ((noinline)) markem(int *a) {
    oft_mark_(&a[1]);
    oft_mark_(&a[3]);
}

void __attribute__ ((noinline)) fn(int *x, int *y) {
    x[1] += 1;
    y[3] -= 1;
}

int main(int argc, char** argv) {
    int *n = malloc(4 * sizeof(int));
    int *m = malloc(4 * sizeof(int));

    markem(n);
    markem(m);
    fn(n, m);
    return 0;
}
