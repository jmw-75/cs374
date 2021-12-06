/* Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 * 
 * Updates by: John White completed on October 12, 2021 @ Calvin Universities Maroon Lab
 * 
 * Updates include: creating a 'Chunks' Structure using parallel loop patterns to speed up
 *                   the computation to parallize the computation through MPI/MPE methods
 */

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <mpe.h>
#include "display.h"
#include <stdlib.h> //malloc


/* compute the Mandelbrot-set function for a given
 *  point in the complex plane.
 *
 * Receive: doubles x and y,
 *          complex c.
 * Modify: doubles ans_x and ans_y.
 * POST: ans_x and ans_y contain the results of the mandelbrot-set
 *        function for x, y, and c.
 */
void compute(double x, double y, double c_real, double c_imag,
              double *ans_x, double *ans_y)
{
        *ans_x = x*x - y*y + c_real;
        *ans_y = 2*x*y + c_imag;
}

/* compute the 'distance' between x and y.
 *
 * Receive: doubles x and y.
 * Return: x^2 + y^2.
 */
double distance(double x, double y)
{
        return x*x + y*y;
}


int main(int argc, char* argv[])
{
    const int  WINDOW_HEIGHT = 900;
    const int  WINDOW_WIDTH  = 1200;
    const double SPACING     = 0.003;

    int        n        = 0,
               ix       = 0,
               iy       = 0,
               button   = 0,
               id       = 0;
    double     x        = 0.0,
               y        = 0.0,
               c_real   = 0.0,
               c_imag   = 0.0,
               x_center = 1.16,
               y_center = -0.10;

    MPE_XGraph graph;

    int numProcesses = -1;
    id = -1;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   if (id==0) {
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                           getDisplay(),
                           -1, -1,
                           WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
   }

   int chunkSize1 = (WINDOW_HEIGHT / numProcesses)+1;          // chunking
   int chunkSize2 = chunkSize1-1;                              // chunking but taking care of remainder
   int remainder = WINDOW_HEIGHT % numProcesses;               // The remaining number not divisible by Num Proc.
   int size = WINDOW_WIDTH * WINDOW_HEIGHT;
   int i=0;
   int * chunkVals    = (int *) malloc( (WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(MPI_INT) ) );   // array for chunk values
   int * chunkFinalVals = (int *) malloc( (WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(MPI_INT) ) ); // final chunks array

   //tracking time
   double startTime = 0.0, totalTime = 0.0;
   startTime = MPI_Wtime();

   if (remainder == 0 || (remainder !=0 && id < remainder)) {        //remainder handling
      ix = 0;                                                        // start at the left
      iy = id*chunkSize1;                                            // start where the last chunk left off.

      if (remainder == 0) {
         chunkSize1--;                                               //if remainder is 0, don't make chunk size 1 bigger
      } 

      for (i = id*chunkSize1*1200; i < (id+1)*chunkSize1*1200; i++) { //run loop over chunks
         if (ix % 1200 == 0) {
            iy ++;
            ix = 0;
         }
         //DONT CHANGE <
         c_real = (ix - 400) * SPACING - x_center;
         c_imag = (iy - 400) * SPACING - y_center;
         x = y = 0.0;
         n = 0;

         while (n < 50 && distance(x, y) < 4.0)
         {
            compute(x, y, c_real, c_imag, &x, &y);
            n++;
         }
         //DONT CHANGE >
         if (n < 50) {
            chunkVals[i] = 1; //YELLOW == 1
         } else {
            chunkVals[i] = 2; //BLACK == 2
         }
         ix++;
      }
   }

   else {
      ix = 0; // start at the left
      iy = ((remainder * chunkSize1) + (chunkSize2 * (id - remainder))); // start where the last chunk left off.

      for (i = ((remainder * chunkSize1) + (chunkSize2 * (id - remainder)))*1200;
           i < ((remainder * chunkSize1) + (chunkSize2 * (id - remainder)) + chunkSize2)*1200;
           i++) //run loop over chunks -- non-remainder loops
      {
         if (ix % 1200 == 0) {
            iy ++;
            ix = 0;
         }
         
         c_real = (ix - 400) * SPACING - x_center;
         c_imag = (iy - 400) * SPACING - y_center;
         x = y = 0.0;
         n = 0;

         while (n < 50 && distance(x, y) < 4.0)
         {
            compute(x, y, c_real, c_imag, &x, &y);
            n++;
         }

         if (n < 50) {
            chunkVals[i] = 1; //YELLOW == 1
         } else {
            chunkVals[i] = 2; //BLACK == 2
         }
         ix++;
      }
   }

   // Reduce the chunks into one using reduce. this provides no value overlap because the values
   // were initialized to null/0
   MPI_Reduce(chunkVals, chunkFinalVals, size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

   totalTime = MPI_Wtime() - startTime; 

   if (id == 0) {
      i = 0;
      for(iy=0; iy<WINDOW_HEIGHT; iy++) {
         for(ix=0; ix<WINDOW_WIDTH; ix++){

            if (chunkFinalVals[i]==1) {
               MPE_Draw_point(graph, ix, iy, MPE_YELLOW);
            }
            else if (chunkFinalVals[i]==2) {
               MPE_Draw_point(graph, ix, iy, MPE_BLACK);
            } else {
	            printf("Blue Pixel! Uh! %d\n",chunkFinalVals[i]);
               MPE_Draw_point(graph, ix, iy, MPE_BLUE);           //shouldl be just black and yellows
            }
            i++; 
         }
      }

      // pause until mouse-click so the program doesn't terminate  
      printf("\nClick in the window to continue...\n");
      printf("Total time for Chunks: %f\n",totalTime);
      MPE_Get_mouse_press( graph, &ix, &iy, &button );
   }

   free(chunkFinalVals);
   free(chunkVals);

   if (id == 0 ) MPE_Close_graphics( &graph );
   MPI_Finalize();
   return 0;
}
