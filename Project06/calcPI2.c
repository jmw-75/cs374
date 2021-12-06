/* calcPI2.c calculates PI using POSIX threads.
 * Since PI == 4 * arctan(1), and arctan(x) is the 
 *  integral from 0 to x of (1/(1+x*x),
 *  the for loop below approximates that integration.
 *
 * Joel Adams, Calvin College, Fall 2013.
 *
 * Usage: ./calcPI2 [numIntervals] [numThreads]
 * 
 * Modified By: John White
 * In order to: improve the programs performance by replacing the mutex zone with a new function that
 *              using a reduction pattern. The function resides in its own header file "pthreadReduction.h"
 *              and uses a barrier function created by professor Adams to syncronize the threads.
 * Date finished: 11/15/21
 * Completed for: CS374 @ Calvin University
 */

#include <stdio.h>                 // printf(), fprintf(), etc.
#include <stdlib.h>                // strtoul(), exit(), ...
#include <pthread.h>               // pthreads
#include <mpi.h>                   // MPI_Wtime()
#include "pthreadReduction.h"  	   // pthreadReductionSum()

// global variables (shared by all threads 
volatile long double pi = 0.0;       // our approximation of PI 
pthread_mutex_t      piLock;         // how we synchronize writes to 'pi' 
long double          intervals = 0;  // how finely we chop up the integration 
unsigned long        numThreads = 0; // how many threads we use 
long double *	     localArraySum;

/* compute PI using the parallel for loop pattern
 * Parameters: arg, a void* 
 * Preconditions: 
 *   - non-locals intervals and numThreads are defined.
 *   - arg contains the address of our thread's ID.
 * Postcondition: non-local pi contains our approximation of PI.
 */
void * computePI(void * arg)
{
    long double   x,
                  width,
                  localSum = 0;
    unsigned long i,
                  threadID = *((unsigned long *)arg);

    width = 1.0 / intervals;

    for(i = threadID ; i < intervals; i += numThreads) {
        x = (i + 0.5) * width;
        localSum += 4.0 / (1.0 + x*x);
    }

    localSum *= width; 
    
    // Place values into sum array in correct position using localSum varible
    localArraySum[threadID] = localSum;

    //ORIGINALLY IN PROGRAM
    /*
    pthread_mutex_lock(&piLock);
    pi += localSum;
    pthread_mutex_unlock(&piLock); 
    */

    // Call to pthreadReductionSum found in "pthreadReduction.h"
    // Function requires pi value, the locally allocated array, the thread ID, and the total number of threads
    pthreadReductionSum(&pi, localArraySum, threadID, numThreads);

    return NULL;
} 

/* process the command-line arguments
 * Parameters: argc, an int; and argv a char**.
 * Postcondition:
 *  - non-locals intervals and numThreads have been defined.
 *     according to the values the user specified when
 *     calcPI2 was invoked.
 */
void processCommandLine(int argc, char ** argv) {
   if ( argc == 3 ) {
      intervals = strtoul(argv[1], 0, 10); 
      numThreads = strtoul(argv[2], 0, 10); 
   } else if ( argc == 2 ) {
      intervals = strtoul(argv[1], 0, 10);
      numThreads = 1;
   } else if ( argc == 1 ) {
      intervals = 1;
      numThreads = 1;
   } else {
      fprintf(stderr, "\nUsage: calcPI2 [intervals] [numThreads]\n\n");
      exit(1);
   }
}
      

int main(int argc, char **argv) {
    pthread_t * threads;            // dynamic array of threads 
    unsigned long  * threadID;      // dynamic array of thread id #s 
    unsigned long i;                // loop control variable 
    double startTime = 0,           // timing variables
           stopTime = 0;

    processCommandLine(argc, argv);

    MPI_Init(&argc, &argv);

    threads = malloc(numThreads*sizeof(pthread_t));
    threadID = malloc(numThreads*sizeof(unsigned long));

    // array needs memory to be allocated for it (used malloc for dynamic allocation)
    // use of malloc noted from https://hpc-tutorials.llnl.gov/posix/samples/mpithreads_mpi.c
    localArraySum = (long double *)malloc( numThreads * sizeof(long double));

    pthread_mutex_init(&piLock, NULL);

    startTime = MPI_Wtime();

    for (i = 0; i < numThreads; i++) {   // fork threads
        threadID[i] = i;
        pthread_create(&threads[i], NULL, computePI, threadID+i);
    }

    for (i = 0; i < numThreads; i++) {   // join them
        pthread_join(threads[i], NULL);
    }
    stopTime = MPI_Wtime();

    printf("Estimation of pi is %32.30Lf in %lf secs\n", pi, stopTime - startTime);
    printf("(actual pi value is 3.141592653589793238462643383279...)\n");
   
    pthread_mutex_destroy(&piLock);
    free(threads);
    free(threadID);
    return 0;
}

