// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 16 17

#include <stdio.h>
extern void oft_mark_(void *);

void __attribute__((noinline)) set_array(int* array) {
    oft_mark_(&(array[0]));
    oft_mark_(&(array[1]));
}

int main() {
    int rank_array[4];
    set_array(rank_array);
    printf("Number is %d\n", rank_array[0] * 3); //mul should be marked
    printf("Number is %d\n", rank_array[1] * 7); //mul should be marked
    return 0;
}
