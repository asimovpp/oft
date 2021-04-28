// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12 

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int size, value;
    oft_mark_(&size);
    int* sizep = &(size);
    value = *sizep * 7;
    printf("Result is %d\n", value);
    return 0;
}
