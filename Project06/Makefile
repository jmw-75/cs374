TARGET = calcPI2
#CC = gcc
CC = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 -I/opt/openmpi/include
LFLAGS = -o $(TARGET) -lpthread -lmpi

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c $(LFLAGS)

clean:
	rm -f $(TARGET) *.o *~ *#


