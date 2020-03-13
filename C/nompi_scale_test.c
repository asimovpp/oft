#include <stdio.h>

//big_num will overflow a 32-bit integer when multiplied by 8
#define BIG_NUM 268435456

int main() {
  int i, size, indirect_overflow, direct_overflow;
  size = 8;

  printf("Size is %d\n", size);

  indirect_overflow = 0;
  for (i = 0; i < size; i++) {
      indirect_overflow += BIG_NUM;
      printf("i = %d; indirect_overflow=%d\n", i, indirect_overflow);
  }

  direct_overflow = size * BIG_NUM;
  
  printf("direct_overflow=%d,\n", direct_overflow);

  return 0;
}
