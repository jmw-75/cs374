/* ompArraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 * 
 * Project continued by John White to use OpenMP and shared-memory parallelism as described in cs374 
 * @ Calvin University, November 24, 2021. The threads should sum the values 
 * in their pieces, using the Parallel Loop and Reduction patterns, then use the master thread to 
 * display the total time, the time spent in I/O, and the time spent summing the array.
 */

#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */
#include <omp.h>

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;

int main(int argc, char * argv[])
{
  int  howMany;
  double sum;
  double * a;
  int id;

//mpi timing
  double totalTime, ioTime, sumTime;
  double startTime, startIoTime, startSumTime;
  startTime = omp_get_wtime();


  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
    exit(1);
  }

  //set numThreads
  omp_set_num_threads (atoi (argv[2]) );
  //Use thread 0
  #pragma omp parallel private(id){
    id = omp_get_thread_num();
    //use thread 0
    if (id == 0) {
      //start time
      startIoTime = omp_get_wtime();
      //read file 
      readArray(argv[1], &a, &homMany);
      //stop IO time
      ioTime = omp_get_wtime() - startIoTime;
    }
  }

  //start sum time
  startSumTime = omp_get_wtime();
  //sum results
  sum = sumArray(a, howMany);
  //stop sum time
  sumTime = omp_get_wtime - startSumTime;
  //stop total time
  totalTime = omp_get_wtime - startTime;

  //display: the total time, the time spent in
  // I/O, and the time spent summing the array.
  printf("PROGRAM RUNTIMES AND SUM\n");
  printf("The sum of the values in the input file '%s' is %g\n", argv[1], sum);
  printf("Total time: %lf\n", totalTime);
  printf("Total IO time: %lf\n", ioTime);
  printf("Total sum time: %lf\n", sumTime);

  // free allocated memory
  free(a);
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

  //parallelize sumArray -- omp threads
  #pragma omp parallel for
  for (i = 0; i < numValues; ++i) {
    #pragma omp atomic
    result += a[i];
  }

  return result;
}

