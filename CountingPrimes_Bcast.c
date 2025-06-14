#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    int np, pid;
    int x, y;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    if (pid == 0) {
        printf("Enter the starting point: \n");
        scanf("%d", &x);
        printf("Enter the end point: \n");
        scanf("%d", &y);
    }

    // Broadcast the range to all processes
    MPI_Bcast(&x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&y, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (pid == 0) {
        int total_primes = 0;
        int partial_count;
        
        // Master process also works on a portion
        int range = y - x + 1;
        int chunk_size = range / np;
        int remainder = range % np;
        
        // Calculate master's portion
        int start = x + pid * chunk_size;
        int end = start + chunk_size - 1;
        if (pid == np - 1) end += remainder;
        
        // Count primes in master's portion
        int count = 0;
        for (int i = start; i <= end; i++) {
            if (is_prime(i)) count++;
        }
        total_primes += count;
        
        // Receive results from workers
        for (int i = 1; i < np; i++) {
            MPI_Recv(&partial_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            total_primes += partial_count;
        }
        
        printf("The range [%d, %d] has %d prime numbers.\n", x, y, total_primes);
    } else {
        // Calculate this worker's portion
        int range = y - x + 1;
        int chunk_size = range / np;
        int remainder = range % np;
        
        int start = x + pid * chunk_size;
        int end = start + chunk_size - 1;
        if (pid == np - 1) end += remainder;
        
        // Count primes in this portion
        int count = 0;
        for (int i = start; i <= end; i++) {
            if (is_prime(i)) count++;
        }
        
        // Send result to master
        MPI_Send(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}