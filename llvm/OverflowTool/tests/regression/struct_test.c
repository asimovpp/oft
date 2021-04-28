// RUN: clang -g -c -O0 -Xclang -disable-O0-optnone -S -emit-llvm %s -o %t1.ll
// RUN: opt -load-pass-plugin %libdir/libLLVMOverflowToolPass.so -aa-pipeline='basic-aa' -passes='oft-overflow-instrumentation' -S -o %t1.instrumented.ll %t1.ll 2> %t1.passout.ll
// RUN: %bindir/check_marked_lines %t1.passout.ll 23 24 

#include <stdio.h>
extern void oft_mark_(void *);

struct scale_vars {
    int rank;
    int size;
};

void __attribute__((noinline)) set_struct_scale_var(struct scale_vars *sv) {
    oft_mark_(&(sv->size));
    oft_mark_(&(sv->rank));
}

int main() {
    int struct_res1, struct_res2;
    struct scale_vars sv;

    set_struct_scale_var(&sv);
    struct_res1 = sv.rank * 3; //multiplication should be marked as a potential scale overflow
    struct_res2 = sv.size * 7; //multiplication should be marked as a potential scale overflow

    printf("struct_res1=%d, struct_res2=%d\n", struct_res1, struct_res2);

    return 0;
}
