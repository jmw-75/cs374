/* calcPi.c calculates PI using the integral of the unit circle.
 * Since the area of the unit circle is PI, 
 *  PI = 4 * the area of a quarter of the unit circle 
 *    (i.e., its integral from 0.0 to 1.0)
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 *
 * Changes made by John White to add parallelization using chunks
 *    while at Calvin University on 10/20/21 for project05 in cs374
 *    High Performence Computing
 */

#include "integral.h"   // integrate()
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // exit()
#include <math.h>       // sqrt() 
#include <mpi.h>        // Paralellization


/* function for unit circle (x^2 + y^2 = 1)
 * parameter: x, a double
 * return: sqrt(1 - x^2)
 */
double f(double x) {
   return sqrt(1.0 - x*x);
}

/* retrieve desired number of trapezoids from commandline arguments
 * parameters: argc: the argument count
 *             argv: the argument vector
 * return: the number of trapezoids to be used.
 */            
unsigned long long processCommandLine(int argc, char** argv) {
   if (argc == 1) {
       return 1;
   } else if (argc == 2) {
//       return atoi( argv[1] );
       return strtoull( argv[1], 0, 10 );
   } else {
       fprintf(stderr, "Usage: ./calcPI [numTrapezoids]");
       exit(1);
   }
}
 

int main(int argc, char** argv) {
   long double approximatePI = 0;
   const long double REFERENCE_PI = 3.141592653589793238462643383279L;
   unsigned long long numTrapezoids = processCommandLine(argc, argv); 

   double startTime, totalTime = 0;
   int id,
       numProcesses.
       start,
       stop = -1;
   int chunkSize,
       chunkSize2,
       remainder;

   // Init MPI
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   // Number of trapazoids processed from command line
   unsigned long long numTrapezoids = processCommandLine(argc, argv);
   
   // From original
   //approximatePI = integrateTrap(0.0, 1.0, numTrapezoids) * 4.0;

   //calculate the chunks
   chunkSize = (int) ceil((double) numTrapezoids / numProcesses);
   remainder = numTrapezoids % numProcesses;

   if (remainder == 0 || (remainder != 0 && id < remainder)) {
      start = id * chunkSize;
      stop = start + chunkSize;
   } else {
      chunkSize2 = chunkSize - 1;
      start = (remainder * chunkSize) + (chunkSize2 * (id - remainder));
      stop = start + chunkSize2;
   }

   startTime = MPI_Wtime();

   localApproximatePI = integrateTrap(0.0, 1.0, numTrapezoids, start, stop, id);
   MPI_Reduce(&localApproximatePI, &totalApproximatePI, 1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
   totalApproximatePI *= 4;

   totalTime = MPI_Wtime() - startTime;

   if ( id == 0 ){
      printf("Using %llu trapezoids, the approximate vs actual values of PI are:\n%.30Lf\n%.30Lf\n",
           numTrapezoids, approximatePI, REFERENCE_PI);
      printf("\nTotal runtime for function to run: %lf\n\n", totalTime);
   }
   
   MPI_Finalize();
   return 0;

}

