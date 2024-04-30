CC = gcc
CCFLAGS = -Wall -fsanitize=address -std=c99 -O2

mysh: mysh.o
	$(CC) $(CFLAGS) mysh.o -o mysh

mysh.o: mysh.c
	$(CC) $(CFLAGS) -c mysh.c

clean:
	rm -f *.o mysh