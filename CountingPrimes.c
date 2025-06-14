#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

bool isPrime(int number)
{
    if (number <= 1)
        return false;
    if (number <= 3)
        return true;

    if (number % 2 == 0 || number % 3 == 0)
        return false;
    for (int i = 5; i * i <= number; i += 6)
    {
        if (number % i == 0 || number % (i + 2) == 0)
            return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    int np, pid;
    int x, y, n_elements_recieved, r;
    int sub_count, index, result;
    int i;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Status status;
    if (pid == 0)
    {
        printf("Enter the starting point: \n");
        scanf("%d", &x);
        printf("Enter the end point: \n");
        scanf("%d", &y);
        int r = (y - x + 1) / np;
        int remainder = (y - x + 1) % np;
        result = 0;
        for (int i = x; i < x + r; i++)
        {
            if (isPrime(i))
                result++;
        }

        for (int i = 1; i < np; i++)
        {
            MPI_Send(&r, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            int start = x + i * r;
            MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (int i = y - remainder + 1; i <= y; i++)
        {
            if (isPrime(x + r * np))
            {
                result++;
            }
        }
        int partial_result;
        for (int i = 1; i < np; i++)
        {
            MPI_Recv(&partial_result, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            result += partial_result;
        }
        
        printf("The count of prime numbers between %d and %d is: %d\n", x, y, result);
    }
    else{
        MPI_Recv(&n_elements_recieved, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        sub_count = 0;
        for (int i = index; i < index + n_elements_recieved; i ++){
            if (isPrime(x + pid * n_elements_recieved)) sub_count ++;
        }
        
        MPI_Send(&sub_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}
