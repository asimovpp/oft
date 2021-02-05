
//big_num will overflow a 32-bit integer when multiplied by 8
#define BIG_NUM 268435457

struct scale_vars {
    int rank;
    int size;
};

extern int global_scale_var;

int __attribute__ ((noinline)) calc_mostly_ok(int r);

int __attribute__ ((noinline)) calc_mostly_ok_pointer(int* r);

int __attribute__ ((noinline)) set_scale_variables(int* r);

void __attribute__ ((noinline)) set_global_scale_var();

void __attribute__ ((noinline)) set_struct_scale_var(struct scale_vars* sv);

int __attribute__ ((noinline)) calc_but_dont_return_scale(int r);
