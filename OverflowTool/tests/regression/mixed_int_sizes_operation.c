// RUN: clang -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: %bindir/run-oft -d %configdir -i %t1.ll detect -oft-print-overflowable=/tmp/%basename_t.passout
// RUN: cat /tmp/%basename_t.passout | wc -l | cmp <(echo 1)

// Do not expect any oveflow 32 bit integer detection since the operation is
// performed with a 64-bit integer

#include <limits.h>

extern void oft_mark_(void *);

int main(int argc, char *argv[]) {
    int rank = argc;
    long long int ret = LONG_MAX;

    oft_mark_(&rank);

    ret += rank;

    return ret;
}
