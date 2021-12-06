/* pthreadReduction.h implements the Barrier pattern using pthreads.
 *
 * John white, Calvin Uni, Fall 2021.
 * 
 * Function of file is to seperate the pthreadReductionSum function in order to call it in CalcPI2.c
 *             The function utilizes pthreadBarrier in order to synchronize the threads. Function requires pi value, 
 *             the locally allocated array, the thread ID, and the total number of threads. Filters through retults 
 *             using the tree method and is only performed if there are values to add.
 */

#include <stdlib.h>
#include <stdio.h>
#include "pthreadBarrier.h"     // pthreadBarrier()


void pthreadReductionSum(volatile long double * totalSum, long double * localArraySum, unsigned long threadID, unsigned long numThreads) {

    // syncronize all the threads allowing for the sum to not have any race conditions
    pthreadBarrier(numThreads);

    // array offset = offset value for the index 
    for (unsigned long arrayOffset = ((unsigned long) numThreads / 2); arrayOffset > 0; arrayOffset>>=1){
        // not all threads are needed
        if (threadID < arrayOffset) {
            //sum up all the values into the 0 thread (reduce)
            localArraySum[threadID] += localArraySum[(int)(threadID + arrayOffset)];
        }
        // re-sync threads
        pthreadBarrier(numThreads);
    }

    // de allocate barrier data
    barrierCleanup();
    // the first item in the array is the sum of all the other items, store first item
    if (threadID == 0){ *totalSum = localArraySum[0]; }
}