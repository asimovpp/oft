// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 11 18
// XFAIL: *

#include <stdio.h>
extern void oft_mark_(void *);
int x;

int __attribute__ ((noinline)) do_calc() {
    return x * 7; //mul should be marked
}

int main() {
    int value;
    oft_mark_(&x);
    value = do_calc();
    printf("Number is %d\n", value * 3); //mul should be marked
    return 0;
}
