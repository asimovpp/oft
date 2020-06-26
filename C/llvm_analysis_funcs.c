#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
//LLVM would be able to change the size of this table as needed
#define TABLE_LEN 100

int values[TABLE_LEN];
FILE *fptr;

void print_val(int v) {
    printf("Value is %d\n", v);
}

void init_vals() {
    for (int i = 0; i < TABLE_LEN; i++) 
        values[i] = -1;
}

void store_max_val(int id, int v) {
    if (v > values[id]) {
        //printf("Instrumentation: Setting val %d to %d\n", id, v);
        values[id] = v;
    }
}

void print_max_vals() {
    int rank = -1;
    int world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int* all_rank_values = malloc(world_size * TABLE_LEN * sizeof(int));
    MPI_Gather(values, TABLE_LEN, MPI_INT, all_rank_values, TABLE_LEN, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        fptr = fopen("overflow_tool_output.txt", "w");
        for (int i = 0; i < world_size; i++) {
            for (int j = 0; j < TABLE_LEN; j++) {
                fprintf(fptr, "Rank %d; Value %d maxed at %d\n", i, j, all_rank_values[i * TABLE_LEN + j]);
            }
        }
        fclose(fptr);
    }
}
