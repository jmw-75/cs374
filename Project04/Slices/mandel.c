/* Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 * 
 * Updates by: John White completed on October 12, 2021 @ Calvin Universities Maroon Lab
 * 
 * Updates include: creating a 'Slices' Structure using parallel loop patterns and 
 *                   the collective communication pattern to combine slices through 
 *                   MPI/MPE methods
 */

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <mpe.h>
#include "display.h"
#include <stdlib.h>


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

   int      n        = 0,
            ix       = 0,
            iy       = 0,
            button   = 0,
            id       = 0;
   double   x        = 0.0,
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

   if (id==0) { //only the one computer has the display ability because security
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                           getDisplay(),
                           -1, -1,
                           WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
   }

   int *rowVals;
   int *printVals;
   MPI_Status status;
   
   rowVals = (int *)malloc(WINDOW_WIDTH*sizeof(int)); //array to put row values in when calculating
   printVals = (int *)malloc(WINDOW_WIDTH*sizeof(int)); //array to put row values in when printing
   
   int temp;
   temp = 1;
   int row;
   
   double startTime = 0.0, totalTime = 0.0;
   startTime = MPI_Wtime();

   if (id==0 && numProcesses > 1){
      //printf("Number of Processes \t %d\n", numProcesses);
      for (temp=1; temp < numProcesses; temp++) {
         //printf("into loop\n");
         for (iy = temp; iy < WINDOW_HEIGHT; iy+=numProcesses) {
            MPI_Recv(printVals, WINDOW_WIDTH,MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            row = status.MPI_TAG;
            //printf("Recieved row:%d\t%d\n",row,ix);
            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
               //printf("Row value %d\n",rowVals[ix]);
               if (printVals[ix]==1) {
                  MPE_Draw_point(graph, ix, row, MPE_RED);
               } else if(printVals[ix]==2) {
                  MPE_Draw_point(graph, ix, row, MPE_ORANGE);
               } else if(printVals[ix]==3) {
                  MPE_Draw_point(graph, ix, row, MPE_YELLOW);
               } else if(printVals[ix]==4) {
                  MPE_Draw_point(graph, ix, row, MPE_GREEN);
               } else if(printVals[ix]==5) {
                  MPE_Draw_point(graph, ix, row, MPE_BLUE);
               }
            }
         }
      }
   }
   
   for (iy = id; iy < WINDOW_HEIGHT; iy+=numProcesses) //run loop over slices
   {
      for (ix = 0; ix < WINDOW_WIDTH; ix++)
      {
         c_real = (ix - 400) * SPACING - x_center;
         c_imag = (iy - 400) * SPACING - y_center;
         x = y = 0.0;
         n = 0;
         
         while (n < 50 && distance(x, y) < 4.0)
         {
            compute(x, y, c_real, c_imag, &x, &y);
            n++;
         }
        
         if (n < 4) {
            rowVals[ix]=1; // store RED as 1
            if (id==0) MPE_Draw_point(graph, ix, iy, MPE_RED);
         } else if ( n < 8 ) {
            rowVals[ix]=2; // store ORANGE as 2
            if (id==0) MPE_Draw_point(graph, ix, iy, MPE_ORANGE);
         } else if ( n < 14 ) {
            rowVals[ix]=3; // store YELLOW as 3
            if (id==0) MPE_Draw_point(graph, ix, iy, MPE_YELLOW);
         } else if ( n < 25 ) {
            rowVals[ix]=4; // store GREEN as 4
            if (id==0) MPE_Draw_point(graph, ix, iy, MPE_GREEN);
         } else {
            rowVals[ix]=5; // store BLUE as 5
            if (id==0) MPE_Draw_point(graph, ix, iy, MPE_BLUE);
         }         
       }

      if (id!=0) { 
         MPI_Send(rowVals, WINDOW_WIDTH, MPI_INT, 0, iy,MPI_COMM_WORLD);
      }
      
   }   

   totalTime = MPI_Wtime() - startTime;
   
   if (id == 0) {
      // pause until mouse-click so the program doesn't terminate
      printf("\nClick in the window to continue...\n");
      printf("Total time for Slices: %f\n",totalTime);
      MPE_Get_mouse_press( graph, &ix, &iy, &button );
   }

   free(rowVals); // avoid memory leaks
   free(printVals); //AVOID MEMORY LEAKS
   if (id==0) MPE_Close_graphics( &graph ); // only id==0 opened graphix, so only it needs clsoe
   MPI_Finalize();
    

   return 0;
}
