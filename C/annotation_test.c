#include <stdio.h>

extern void oft_mark(void*);

int main() {
    int a_number = 42; 
    oft_mark(&a_number);
  
    printf("Local num is %d\n", a_number * 7);
    return 0;
}