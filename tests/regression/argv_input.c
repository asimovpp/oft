// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 12
// XFAIL: *

#include <stdio.h>

int main(int argc, char *argv[]) {
    int user_input;
    if (argc == 2)
        user_input = atoi(argv[1]);
    printf("Number is %d\n", user_input * 7); //mul should be marked
    return 0;
}
// there is no syntax currently to mark positional arguments automatically via config
