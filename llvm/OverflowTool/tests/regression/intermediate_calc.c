// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: grep ".*given to.*Line 13.*" %t1.passout.ll
// RUN: grep ".*given to.*Line 14.*" %t1.passout.ll

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int size, i1, i2, i3;
    oft_mark_(&size);
    i1 = 3 * size;
    i2 = 7 + i1;
    i3 = 9 * i2;
    printf("Results are %d %d %d\n", i1, i2, i3);
    return 0;
}

