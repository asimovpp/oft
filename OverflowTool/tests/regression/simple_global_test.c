// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 15 

#include <stdio.h>
extern void oft_mark_(void *);

int global_value;
void __attribute__ ((noinline)) set_global() {
    oft_mark_(&global_value);
}

int main() {
    set_global();
    printf("Number is %d\n", global_value * 7); //mul should be marked
    return 0;
}
