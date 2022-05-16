// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 10
// XFAIL: *

#include <stdio.h>
extern void oft_mark_(void *);

int __attribute__ ((noinline)) do_calc(int r) {
    int x = r * 7; //mul should be marked
    return 42;
}

int main() {
    int size, value;
    oft_mark_(&size);
    value = do_calc(size);
    printf("Number is %d\n", value * 3);
    return 0;
}
// function returns are followed if a scale variable is traced into an argument,
// but that does not always mean that the return value should be traced
