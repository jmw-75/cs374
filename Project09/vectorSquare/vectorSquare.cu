/**
 * Copyright 1993-2013 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 * Extended for use in CS 374 at Calvin College by Joel C. Adams.
 */

/**
 * Vector square of A = C.
 *
 * This sample is a very basic sample that implements element by element
 * vector square. It is the same as the sample illustrating Chapter 2
 * of the programming guide with some squares like error checking.
 */

#include <stdio.h>

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

// For the runtimes using OpenMP
#include <omp.h>

/**
 * CUDA Kernel Device code
 *
 * Computes the vectorSquare A * A = C. 
 * The 3 vectors have the same number of elements numElements.
 */
__global__
void vectorSquare(const float *A, float *C, unsigned long numElements)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < numElements)
    {
        C[i] = A[i] * A[i];
    }
}

void checkErr(cudaError_t err, const char* msg) 
{
    if (err != cudaSuccess)
    {
        fprintf(stderr, "%s (error code %d: '%s')!\n", msg, err, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

/**
 * Host main routine
 */
int main(int argc, char** argv)
{
    // Timing varibles
    double startCuda, finalCuda;
    double startSequential, finalSequential;
    double startSum, finalSum;
    double startHostToDev, finalHostToDev;
    double startDevToHost, finalDevToHost;

    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Print the vector length to be used, and compute its size
    unsigned long numElements = 50000;
    if (argc == 2) {
      numElements = strtoul( argv[1] , 0, 10 );
    }
    size_t size = numElements * sizeof(float);
    printf("[Vector Square of %lu elements]\n", numElements);

    // Allocate the host input vectors A
    float * h_A = (float *)malloc(size);

    // Allocate the host output vector C
    float * h_C = (float *)malloc(size);

    // Verify that allocations succeeded
    if (h_A == NULL || h_C == NULL)
    {
        fprintf(stderr, "Failed to allocate host vectors!\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the host input vectors
    for (int i = 0; i < numElements; ++i)
    {
        h_A[i] = rand()/(float)RAND_MAX;
    }

    // 1a. Allocate the device input vectors A
    float * d_A = NULL;
    err = cudaMalloc((void **)&d_A, size);
    checkErr(err, "Failed to allocate device vector A");

    // 1.b. Allocate the device output vector C
    float * d_C = NULL;
    err = cudaMalloc((void **)&d_C, size);
    checkErr(err, "Failed to allocate device vector C");

    // 2. Copy the host input vector A in host memory 
    //     to the device input vectors in device memory
    startCuda = omp_get_wtime();                                            //start Cuda total time
    startHostToDev = omp_get_wtime();                                       //start host to dev time
    printf("Copy input data from the host memory to the CUDA device\n");
    err = cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    checkErr(err, "Failed to copy device vector A from host to device");
    finalHostToDev = omp_get_wtime() - startHostToDev;                      //stop host to dev time

    // 3. Launch the Vector square CUDA Kernel
    startSum = omp_get_wtime();                                             //start computing of sum Time
    int threadsPerBlock = 256;
    int blocksPerGrid =(numElements + threadsPerBlock - 1) / threadsPerBlock;
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);
    vectorSquare<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_C, numElements);
    err = cudaGetLastError();
    checkErr(err, "Failed to launch vectorSquare kernel");
    finalSum = omp_get_wtime() - startSum;                                  //stop computing of sum time


    // 4. Copy the device result vector in device memory
    //     to the host result vector in host memory.
    startDevToHost = omp_get_wtime();                                       //start Dev to Host time
    printf("Copy output data from the CUDA device to the host memory\n");
    err = cudaMemcpy(h_C, d_C, size, cudaMemcpyDeviceToHost);
    checkErr(err, "Failed to copy vector C from device to host");
    finalDevToHost = omp_get_wtime() - startDevToHost;                      //stop Dev to Host
    finalCuda = omp_get_wtime() - startCuda;                                //stop Cuda time

    // Verify that the result vector is correct
    for (int i = 0; i < numElements; ++i)
    {
        if (fabs( h_A[i] * h_A[i] - h_C[i] ) > 1e-5)
        {
            fprintf(stderr, "Result verification failed at element %d!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("CUDA test PASSED\n");
//    printf("CUDA time: %lf\n", stopTime-startTime); 
    printf("Host to dev time: %lf\n", finalHostToDev);
    printf("Computation Sum time: %lf\n", finalSum);
    printf("Dev to Host time: %lf\n", finalDevToHost);
    printf("Cuda runtime: %lf\n", finalCuda);

    // Free device global memory
    err = cudaFree(d_A);
    checkErr(err, "Failed to free device vector A");

    err = cudaFree(d_C);
    checkErr(err, "Failed to free device vector C");

    // repeat the computation sequentially
    startSequential = omp_get_wtime();                              //start Sequential time
    for (int i = 0; i < numElements; ++i)
    {
       h_C[i] = h_A[i] * h_A[i];
    }
    finalSequential = omp_get_wtime() - startSequential;            //stop sequential time
    printf("Sequential time: %lf\n", finalSequential);


    // verify again
    for (int i = 0; i < numElements; ++i)
    {
        if (fabs( h_A[i] * h_A[i] - h_C[i] ) > 1e-5)
        {
            fprintf(stderr, "Result verification failed at element %d!\n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("\nNormal test PASSED\n");
//    printf("Normal time: %lf\n", stopTime-startTime); 
    
    // Free host memory
    free(h_A);
    free(h_C);

    // Reset the device and exit
    err = cudaDeviceReset();
    checkErr(err, "Unable to reset device");

    printf("Done\n");
    return 0;
}

