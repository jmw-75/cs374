/* Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams.
 *  
 * Updates by: John White completed on October 12, 2021 @ Calvin Universities Maroon Lab
 * 
 * Updates include: creating a Master-Worker Structure using Message-Passing patterns
 *                   through MPI/MPE methods
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

   if (id==0){
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                        getDisplay(),
                        -1, -1,
                        WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
   }
 
   int *rowVals;                                            // pointer for array
   int *printVals;                                          // pointer for array
   MPI_Status status;                                       // represents status of message  
   rowVals = (int *)malloc(WINDOW_WIDTH*sizeof(int));       // array for row vala when calculating
   printVals = (int *)malloc(WINDOW_WIDTH*sizeof(int));     // array for row vals when printing

   int toCalc = numProcesses;
   int calcPlease;
   int row;
   int rowsFinished=numProcesses;
   int worker;

   double startTime = 0.0, totalTime = 0.0;
   startTime = MPI_Wtime();

   if (numProcesses==1){                                    // master does it all
      for (iy = 0; iy < WINDOW_HEIGHT; iy++)                //run loop over slices
      {
         for (ix = 0; ix < WINDOW_WIDTH; ix++)
         {
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
               MPE_Draw_point(graph, ix, iy, MPE_YELLOW);
            } else {
               MPE_Draw_point(graph, ix, iy, MPE_BLACK);
            }         
         }
      }
   } 
   
   //master worker approach, each process does the row that matches its id
   else {                                                
      iy = id;                                           
         for (ix = 0; ix < WINDOW_WIDTH; ix++) {
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
         
            // Only print with the Master (ie. when id == 0)
            if (n < 50) {
               rowVals[ix]=69; // store PINK as 69
               if (id==0) MPE_Draw_point(graph, ix, iy, MPE_YELLOW); 
            } else {
               rowVals[ix]=87; // store BLACK as 87
               if (id==0) MPE_Draw_point(graph, ix, iy, MPE_BLACK); 
            }         
         }

      int done = -1;
      toCalc = numProcesses;
      if (id == 0) {
         rowsFinished = 0;
         while(rowsFinished != 899){
            MPI_Recv(printVals, WINDOW_WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            row = status.MPI_TAG;
            worker = status.MPI_SOURCE;
            rowsFinished++;

            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
               if (printVals[ix]==69) {
                  MPE_Draw_point(graph, ix, row, MPE_YELLOW);
               } else if(printVals[ix]==87) {
                  MPE_Draw_point(graph, ix, row, MPE_BLACK);
               }
            }

            
            if (toCalc < WINDOW_HEIGHT) {
               MPI_Send(&toCalc, 1, MPI_INT, worker, iy, MPI_COMM_WORLD);
            } else {
               MPI_Send(&done, 1, MPI_INT, worker, iy, MPI_COMM_WORLD);
            }
            toCalc++;
            if (rowsFinished == 899) {
               break;
            }
         }

      }

      else if(id !=0) {
         while(iy!=-1) {
            //printf("Sending row %d\t%d\tfrom\t%d\n",iy,ix, id);
            MPI_Send(rowVals, WINDOW_WIDTH, MPI_INT, 0, iy, MPI_COMM_WORLD);
            //printf("row %d\t%d\tfrom\t%d\tsent\n",iy,ix, id);
            MPI_Recv(&calcPlease, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            iy = calcPlease; //set the row value to recieved row from master

            //printf("row %d\trecieved\n",iy);

            if (iy==-1) break; //breaks while loop when it gets the STOP! signal
            
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
               //printf("Made it here %d\n", id);
               if (n < 50) {
                  rowVals[ix]=69; // store PINK as 69
               } else {
                  rowVals[ix]=87; // store BLACK as 87
               }         
            }
         }
      }
   }
  
   // all the sending and recieving should be included in the total time.
   totalTime = MPI_Wtime() - startTime; 

   // From OG
   if (id == 0) {
      // pause until mouse-click so the program doesn't terminate
      printf("Total time for Master-Worker: %f",totalTime);
      printf("\nClick in the window to continue...\n");
      MPE_Get_mouse_press( graph, &ix, &iy, &button );
   }

   free(rowVals);
   free(printVals);
   if (id==0) MPE_Close_graphics( &graph );
   MPI_Finalize();
    

   return 0;
}
