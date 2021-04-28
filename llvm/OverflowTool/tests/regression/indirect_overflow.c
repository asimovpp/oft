// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: grep ".*given to.*Line 14.*" %t1.passout.ll

#include <stdio.h>
extern void oft_mark_(void *);


int main() {
    int size;
    int indirect_overflow = 0;
    oft_mark_(&size);
    for (int i = 0; i < size; i++)
        indirect_overflow += 7 + i; //multiplication should be marked as a potential scale overflow
    printf("Number is %d\n", indirect_overflow);
    return 0;
}