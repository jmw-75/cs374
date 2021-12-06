#!/bin/bash
# Example with 1 nodes, 8 processes each = 8 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 1
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=8
#

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./mpiArraySum /home/cs/374/exercises/07/10m.txt
