// RUN: clang -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: %bindir/run-oft -p -d %configdir -i %t1.ll annotate 2>&1 | grep -v %t1.ll | wc -l | cmp <(echo 1)

#include <stdio.h>

int main() {
    int a_number = 0;
    scanf("%d", &a_number);

    printf("Local num is %d\n", a_number * 7);

    return 0;
}
