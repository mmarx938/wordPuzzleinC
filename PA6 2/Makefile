CC=gcc
CFLAGS=-I. -g -w -pthread -lpthread
OBJ = puzzle.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

puzzle: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
