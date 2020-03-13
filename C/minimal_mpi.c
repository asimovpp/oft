#include <stdio.h>
#include <mpi.h>

int main() {
  MPI_Init(NULL, NULL);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from rank %d\n", rank);
  MPI_Finalize();
  return 0;
}
