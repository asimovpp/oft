#include <stdio.h>
#include <mpi.h>

//big_num will overflow a 32-bit integer when multiplied by 8
#define BIG_NUM 268435456

int main() {
  MPI_Init(NULL, NULL);
  int i, rank, size, indirect_overflow, direct_overflow, mostly_ok_int;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  printf("Hello from rank %d\n", rank);
  printf("Hello from rank %d\n", size);

  indirect_overflow = 0;
  for (i = 0; i < size; i++) 
      indirect_overflow += BIG_NUM;
  
  direct_overflow = size * BIG_NUM;
  mostly_ok_int = rank * BIG_NUM;
  
  printf("Rank %d; mostly_ok_int=%d, direct_overflow=%d, indirect_overflow=%d\n", 
          rank, mostly_ok_int, direct_overflow, indirect_overflow);

  MPI_Finalize();
  return 0;
}
