#!/bin/bash
# Example with 1 nodes, 4 processes each = 4 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 1
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=4
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./mpiArraySum /home/cs/374/exercises/07/10k.txt
