#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv);
	int pid, np;
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	if (np == 1) {
        printf("Error: Need at least 1 slave process.\n");
        MPI_Finalize();
        return 1;
    }
	int arrSize;
	int* arr = NULL;
	int sliceSize;


	if (pid == 0){
		printf("Hello from master process.\n");
		printf("Number of slave processes is %d\n", np - 1);
		printf("Enter size of the array: \n");
		//fflush(stdout);
		scanf("%d", &arrSize);
		sliceSize = arrSize / (np - 1);
		printf("Enter the desired array: \n");
		arr = malloc(arrSize * sizeof(int));
		for (int i = 0; i < arrSize; i ++){
			scanf("%d", &arr[i]);
		}
		int* tmp = malloc(sliceSize * sizeof(int));
		for (int i = 1; i < np; i ++){
			if ( i == np - 1 ){
				int rest = arrSize - sliceSize * (i - 1);
				MPI_Send(&rest, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
				MPI_Send(&arr[sliceSize * (i - 1)], rest, MPI_INT, i, 2, MPI_COMM_WORLD);
			}
			else{
				MPI_Send(&sliceSize, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
				MPI_Send(&arr[(i - 1) * sliceSize], sliceSize, MPI_INT,i , 2, MPI_COMM_WORLD);
			}
		}
		
		int gMax = -9999;
		for (int i = 1; i < np ; i ++)
		{
			int lMax;
			MPI_Recv(&lMax, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
			if (lMax >= gMax) gMax = lMax;
		}
		printf("The global maximum is : %d\n", gMax);
	}

	else{
		int* tmpArr = malloc(sliceSize * sizeof(int));
		MPI_Recv(&sliceSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(tmpArr, sliceSize, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
		int max = -9999;
		for (int i = 0; i < sliceSize; i ++){
			if (tmpArr[i] > max) max = tmpArr[i];
		}
		MPI_Send(&max, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}

	
