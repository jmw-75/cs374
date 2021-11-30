OBJ = mpiArraySum
CC = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 -lm
LFLAGS = -o $(OBJ) -fopenmp -lm

$(OBJ): $(OBJ).c
	$(CC) $(CFLAGS) $(OBJ).c $(LFLAGS)

clean:
	rm -f a.out $(OBJ)