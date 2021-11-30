/* mpiArraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 * 
 * Project continued by John White to use using MPI as described in cs374 
 * @ Calvin University, November 26, 2021. The master process reads in the array and uses the Scatter pattern 
 * to distribute a piece of the array to every process each process should sum its piece; then the program uses 
 * the Reduction pattern to then display the total time, the time spent in I/O, the time to scatter the values, 
 * and the time spent summing the array.
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <mpi.h>
#include <math.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;

int main(int argc, char * argv[])
{
  int  howMany;
  double sum;
  double * a;
  double * localArray;
  int id, numProcesses, remainder;
  unsigned int localChunk;
  double localSum;

  //mpi timing
  double totalTime, ioTime, scatterTime, sumTime;
  double startTime, startIoTime, startScatterTime, startSumTime;

  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
    exit(1);
  }

  //Initialize MPI and start time
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  startTime = MPI_Wtime();
  
  //create a master only read
  if (id ==0) {
    //start time
    startIoTime = = MPI_Wtime();
    readArray(argv[1], &a, &howMany);
    //how long IO took
    ioTime = MPI_Wtime() - startIoTime
  }

  //based on example slide 16.17 (make sure correct number)
  MPI_Bcast(&howMany, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  //create local chunk and array
  localChunk = (unsigned int)(howMany / numProcesses);
  localArray = malloc(sizeof(double) * localChunk);

  //start scatter time
  startScatterTime = MPI_Wtime();
  MPI_Scatter(a, localChunk, MPI_DOUBLE, localArray, localChunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  scatterTime = MPI_Wtime() - startScatterTime;  //end scatter time

  //start sum time
  startSumTime = MPI_Wtime();
  localSum = sumArray(localArray, localChunk);

  //Reduce from local sums to 'sum'
  MPI_Reduce(&localSum, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  // only master
  if (id == 0){
    remainder = howMany % numProcesses;
    //add to remainder if there is one
    if (remainder != 0){
        for (int i = 0;i < remainder; i++){
          sum += a[(localChunk * numProcesses) + i];
        }
    }
    sumTime = MPI_Wtime() - startSumTime;
    totalTime = MPI_Wtime() - startTime;

    //display: the total time taken by the program; the time spent in I/O; 
    //the time to scatter the values; and the time to sum the array
    printf("PROGRAM RUNTIMES AND SUM\n");
    printf("The sum of the values in the input file '%s' is %g\n", argv[1], sum);
    printf("Total time: %lf\n", totalTime);
    printf("Total IO time: %lf\n", ioTime);
    printf("Total scatter time: %lf\n", scatterTime);
    printf("Total sum time: %lf\n", sumTime);
  }

  //allocated memory with process 0 (master)
  if (id == 0){
    //no leaky mem
    free(a); 
  }
  
  //no leaky mem
  free(localArray);
  MPI_Finalize();
  return 0;
}

/* readArray fills an array with values from a file.
 * Receive: fileName, a char*,
 *          a, the address of a pointer to an array,
 *          n, the address of an int.
 * PRE: fileName contains N, followed by N double values.
 * POST: a points to a dynamically allocated array
 *        containing the N values from fileName
 *        and n == N.
 */

void readArray(char * fileName, double ** a, int * n) {
  int count, howMany;
  double * tempA;
  FILE * fin;

  fin = fopen(fileName, "r");
  if (fin == NULL) {
    fprintf(stderr, "\n*** Unable to open input file '%s'\n\n",
                     fileName);
    exit(1);
  }

  fscanf(fin, "%d", &howMany);
  tempA = calloc(howMany, sizeof(double));
  if (tempA == NULL) {
    fprintf(stderr, "\n*** Unable to allocate %d-length array",
                     howMany);
    exit(1);
  }

  for (count = 0; count < howMany; count++)
   fscanf(fin, "%lf", &tempA[count]);

  fclose(fin);

  *n = howMany;
  *a = tempA;
}

/* sumArray sums the values in an array of doubles.
 * Receive: a, a pointer to the head of an array;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

double sumArray(double * a, int numValues) {
  int i;
  double result = 0.0;

  for (i = 0; i < numValues; ++i) {
    //parallelize sumArray
    result += *a;
    a++;
  }

  return result;
}

