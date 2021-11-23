// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load %libdir/libLLVMOverflowToolPass.so -oft-trace-loops -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12 13 14

#include <stdio.h>
extern void oft_mark_(void *);

int main() {
    int size, temp;
    int indirect_overflow = 0;
    oft_mark_(&size);
    for (int i = 0; i < size; i++) {
        temp = 7 + i; //add should be marked
        indirect_overflow += temp; //add should be marked
    }
    printf("Number is %d\n", indirect_overflow);
    return 0;
}
