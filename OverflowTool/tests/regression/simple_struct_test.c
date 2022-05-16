// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 20 21 

#include <stdio.h>
extern void oft_mark_(void *);

struct scale_vars {
    int rank;
    int size;
};

int main() {
    int struct_res1, struct_res2;
    struct scale_vars sv;

    oft_mark_(&(sv.size));
    oft_mark_(&(sv.rank));
    
    struct_res1 = sv.rank * 3; //mul should be marked
    struct_res2 = sv.size * 7; //mul should be marked

    printf("struct_res1=%d, struct_res2=%d\n", struct_res1, struct_res2);
    return 0;
}
