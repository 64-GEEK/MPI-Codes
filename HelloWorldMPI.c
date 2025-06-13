#include <mpi.h>
#include <stdio.h>

int main(int argc, char* argv[]){
    int pid, np;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np); // to get the size of the communicator ----> MPI_COMM_WORLD is the default communicator
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Hello World from processor %s, rank %d out of %d processors\n", processor_name, pid, np);
    MPI_Finalize();
    return 0;
}