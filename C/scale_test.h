
//big_num will overflow a 32-bit integer when multiplied by 8
#define BIG_NUM 268435457

extern int global_scale_var;

int __attribute__ ((noinline)) calc_mostly_ok(int r);

int __attribute__ ((noinline)) calc_mostly_ok_pointer(int* r);

void __attribute__ ((noinline)) set_global_scale_var();
