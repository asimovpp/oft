// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 9 

#include <stdio.h>
extern void oft_mark_(void *);

int __attribute__ ((noinline)) do_calc_ptr(int* r) {
    return *r * 7; //mul should be marked
}

int main() {
    int size, value;
    oft_mark_(&size);
    int* sizep = &(size);
    value = do_calc_ptr(sizep);
    printf("Result is %d\n", value);
    return 0;
}
