// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load %libdir/libLLVMOverflowToolPass.so -oft-annotation-files=%configdir"/linux_libc_io.cfg" -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 11 

#include <stdio.h>

int main() {
    int a_number = 0;
    scanf("%d", &a_number);

    printf("Local num is %d\n", a_number * 7); //mul should be marked

    return 0;
}

// old run: %bindir/run-oft -p -d %configdir -i %t1.ll annotate 2>&1 | grep -v %t1.ll | wc -l | cmp <(echo 1)
