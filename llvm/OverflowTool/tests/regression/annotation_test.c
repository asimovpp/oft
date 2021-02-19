// RUN: clang -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: %bindir/run-oft-manual-annot-sel-print %t1.ll 2>&1 | grep -v %t1.ll | wc -l | cmp <(echo 1)

#include <stdio.h>

extern void oft_mark_(void *);

int main() {
    int a_number = 42;
    oft_mark_(&a_number);

    printf("Local num is %d\n", a_number * 7);
    return 0;
}
