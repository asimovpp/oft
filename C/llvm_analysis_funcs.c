#include <stdio.h>
#include <mpi.h>
//LLVM would be able to change the size of this table as needed
#define TABLE_LEN 100

int values[TABLE_LEN];

void print_val(int v) {
    printf("Value is %d\n", v);
}

void init_vals() {
    for (int i = 0; i < TABLE_LEN; i++) 
        values[i] = -1;
}

void store_max_val(int id, int v) {
    if (v > values[id]) {
        printf("Instrumentation: Setting val %d to %d\n", id, v);
        values[id] = v;
    }
}

void print_max_vals() {
    int rank = -1;
    //MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for (int i = 0; i < TABLE_LEN; i++) 
        printf("Rank %d; Value %d maxed at %d\n", rank, i, values[i]);
}
