#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int *values;
int TABLE_LEN;
FILE *fptr;
int mpi_finalize_has_been_called;
int mpi_rank;
int instr_active_after_finalize;

void __attribute__ ((nodebug)) oft_print_val(int v) {
    printf("Value is %d\n", v);
}

void oft_final_check(void) {
    if (instr_active_after_finalize == 1) {
        if (mpi_rank == 0 || mpi_rank == -1) {
            fptr = fopen("oft_debug.txt", "w");
            fprintf(fptr, "Some instrumented scale variables were written to after MPI_Finalize had been called.\n");
            fclose(fptr);
        }
    }
}

void __attribute__ ((nodebug)) oft_init_vals(int table_len) {
    TABLE_LEN = table_len;
    values = malloc(TABLE_LEN * sizeof(int));
    for (int i = 0; i < TABLE_LEN; i++) 
        values[i] = -1;

    if (atexit(oft_final_check) != 0) {
        printf("Failed to register instrumentation finalisation function. Exiting.\n");
        exit(1);
    }

    mpi_finalize_has_been_called = 0;
    mpi_rank = -1;
    instr_active_after_finalize = 0;
}

void __attribute__ ((nodebug)) oft_store_max_val(int id, int v) {
    if (v > values[id]) {
        //printf("Instrumentation: Setting val %d to %d\n", id, v);
        values[id] = v;
    }

    if (mpi_finalize_has_been_called == 1)
      instr_active_after_finalize = 1;
}

void __attribute__ ((nodebug)) oft_print_max_vals() {
    int rank = -1;
    int world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    mpi_rank = rank;
    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int* all_rank_values = malloc(world_size * TABLE_LEN * sizeof(int));
    MPI_Gather(values, TABLE_LEN, MPI_INT, all_rank_values, TABLE_LEN, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        fptr = fopen("oft_output.txt", "w");
        for (int i = 0; i < world_size; i++) {
            for (int j = 0; j < TABLE_LEN; j++) {
                fprintf(fptr, "Rank %d; Value %d maxed at %d\n", i, j, all_rank_values[i * TABLE_LEN + j]);
            }
        }
        fclose(fptr);
    }

    mpi_finalize_has_been_called = 1;
}
