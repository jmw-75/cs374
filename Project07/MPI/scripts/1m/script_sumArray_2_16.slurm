#!/bin/bash
# Example with 2 nodes, 16 processes each = 32 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 2
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=16
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./mpiArraySum /home/cs/374/exercises/07/1m.txt
