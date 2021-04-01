// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: grep ".*given to.*Line 11.*" %t1.passout.ll

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int size;
    oft_mark_(&size);
    printf("Number is %d\n", size * 7); //multiplication should be marked as a potential scale overflow
    return 0;
}
