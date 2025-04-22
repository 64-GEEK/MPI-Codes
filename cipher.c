#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

char charEncode(char c)
{
    if (c >= 'a' && c <= 'z')
        return 'a' + (c - 'a' + 3) % 26;
    else if (c >= 'A' && c <= 'Z')
        return 'A' + (c - 'A' + 3) % 26;
    else
        return c;
}

char charDecode(char c)
{
    if (c >= 'a' && c <= 'z')
        return 'a' + (c - 'a' - 3 + 26) % 26;
    else if (c >= 'A' && c <= 'Z')
        return 'A' + (c - 'A' - 3 + 26) % 26;
    else
        return c;
}

void stringParser(char *str, int len, int mode)
{
    for (int i = 0; i < len; i++)
    {
        if (mode == 1)
        {
            str[i] = charEncode(str[i]);
        }
        else if (mode == 2)
        {
            str[i] = charDecode(str[i]);
        }
        else
        {
            printf("Invalid Operation Code!");
        }
    }
}
int main(int argc, char *argv[])
{

    MPI_Init(&argc, &argv);
    int pid, np;
    MPI_Status status;
    char *input_string = NULL;
    int puffer_len = 0;
    int input_mode = 0;
    int mode = 0; // here is the mode decrypt or encrypt

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (np <= 1)
    {
        printf("Error: At least 2 processes are required.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    if (pid == 0)
    {
        printf("Welcome to the Encryption/Decryption Program!\n");
        printf("Choose input mode:\n");
        printf("1. Console\n");
        printf("2. Input File\n");
        scanf("%d", &input_mode);
        getchar();

        if (input_mode == 1)
        {
            printf("Input: \n");
            input_string = malloc(1000 * sizeof(char));
            fgets(input_string, 1000, stdin);
            puffer_len = strlen(input_string);
            // عشان لو علي اكتر من سطر
            if (input_string[puffer_len - 1] == '\n')
            {
                input_string[puffer_len - 1] = '\0';
                puffer_len--;
            }
        }
        else if (input_mode == 2)
        {
            printf("Enter input file's name: \n");
            char filename[100];
            scanf("%s", filename);
            getchar();
            FILE *input = fopen("input.txt", "r");
            input_string = malloc(1000 * sizeof(char));
            if (input == NULL)
            {
                printf("Input File Failed to Open!");
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 1;
            }
            if (fgets(input_string, 1000, input) == NULL)
            {
                printf("Error reading file!\n");
                fclose(input);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 1;
            }
            puffer_len = strlen(input_string);
            if (puffer_len > 0, input_string[puffer_len - 1] == '\n')
            {
                input_string[puffer_len - 1] == '\0';
                puffer_len--;
            }
        }
            else
            {
                printf("Invalid Input Mode!\n");
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 1;
                // MPI_Recv(&mode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // MPI_Recv(&input_mode, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // MPI_Recv(&puffer_len, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            printf("Choose :\n");
            printf("1. Encrypt\n");
            printf("2. Decrypt\n");
            scanf("%d", &mode);
            getchar();
            // for (int i = 1; i < np; i++)
            // {
            //     MPI_Send(&mode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            //     MPI_Send(&input_mode, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            //     MPI_Send(&puffer_len, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
            // }
    }

    int slice_size = puffer_len / (np - 1);                                // calculates the base size of each slice
    int remainder = puffer_len % (np - 1);                                 // the leftover characters cannot be divided evenly
    int local_size = ((np - 1) < remainder) ? slice_size + 1 : slice_size; // the size of the slice
    char *local_str = (char *)malloc((local_size + 1) * sizeof(char));

    if (pid == 0)
    {
        for (int i = 1; i < np; i++)
        {
            MPI_Send(&mode, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&local_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&input_string[i * slice_size], local_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
        for (int i = 0; i < local_size; i++)
        {
            stringParser(&input_string[i * slice_size], local_size, mode);
        }
        // Handle the remainder
        if (remainder > 0)
        {
            for (int i = 0; i < remainder; i++)
            {
                stringParser(&input_string[(np - 1) * slice_size + i], 1, mode);
            }
        }
        // Combine the results
        for (int i = 1; i < np; i++)
        {
            MPI_Recv(&input_string[i * slice_size], local_size, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("The final result is: %s\n", input_string);
    }

    else
    {
        MPI_Recv(&mode, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        char *local_str = (char *)malloc((local_size + 1) * sizeof(char));
        if (local_str == NULL)
        {
            printf("Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        MPI_Recv(local_str, local_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        stringParser(local_str, local_size, mode);
        MPI_Send(local_str, local_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        free(local_str);
    }
    MPI_Finalize();
    free(input_string);
    return 0;
}
