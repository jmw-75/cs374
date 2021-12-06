/* masterServer.c
 * ... illustrates the basic master-worker pattern in MPI ...
 * Joel Adams, Calvin College, November 2009.
 */

#include <stdio.h>
#include <mpi.h>
#include <math.h>   // sqrt()

int main(int argc, char** argv) {
  int id = -1, numWorkers = -1, length = -1;
  float sendValue = -1, receivedValue = -1;
  char hostName[MPI_MAX_PROCESSOR_NAME];
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);
  MPI_Get_processor_name (hostName, &length);

  sendValue = sqrt(id);

  if ( id == 0 ) {  // process 0 is the master 
    printf("Greetings from the master, # %d (%s) of %d processes\n",
            id, hostName, numWorkers);
    MPI_Send(&sendValue, 1, MPI_CHAR, id+1, 1, MPI_COMM_WORLD);
    MPI_Recv(&receivedValue, 1, MPI_FLOAT, id-1, 1, 
                       MPI_COMM_WORLD, &status);
    printf("Process %d of %d computed %f and received %f\n",
                id, numWorkers, sendValue, receivedValue);
  } else {          // processes with ids > 0 are workers 
    printf("Greetings from a worker, # %d (%s) of %d processes\n",
            id, hostName, numWorkers);
    MPI_Recv(&receivedValue, 1, MPI_FLOAT, id-1, 1, 
                       MPI_COMM_WORLD, &status);
    MPI_Send(&sendValue, 1, MPI_FLOAT, (id+1) % numWorkers, 1, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}

