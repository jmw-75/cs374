TARGET = vectorMult
CC     = g++
CFLAGS = -Wall -ansi -pedantic -std=c99
LFLAGS = -o $(TARGET) -lgomp
NVCC = nvcc
# Use the next line in the Gold Lab (Ampere architecture)
#CUDA_FLAGS = -gencode arch=compute_86,code=sm_86 
# Use the next line in the Maroon Lab (Pascal architecture)
CUDA_FLAGS = -gencode arch=compute_61,code=sm_61 

$(TARGET): $(TARGET).cu
	$(NVCC) $(CUDA_FLAGS) $(TARGET).cu $(LFLAGS)

clean: 
	rm -rf *o $(TARGET)
