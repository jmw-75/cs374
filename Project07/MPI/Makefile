OBJ = mpiArraySum
CC = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 -g
LFLAGS = -o $(OBJ) -fopenmp -lm -g

$(OBJ): $(OBJ).c
	$(CC) $(CFLAGS) $(OBJ).c $(LFLAGS)

clean:
	rm -f a.out $(OBJ)
