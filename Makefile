CC = gcc
CFLAGS = -Wall -g

minget: minget.o min.o
	$(CC) $(CFLAGS) -o minget minget.o min.o

minls: minls.o min.o
	$(CC) $(CFLAGS) -o minls minls.o min.o

minget.o: minget.c
	$(CC) $(CFLAGS) -c minget.c

minls.o: minls.c
	$(CC) $(CFLAGS) -c minls.c

min.o: min.c
	$(CC) $(CFLAGS) -c min.c

clean: 
	rm minget minget.o minls minls.o

all: minls minget

test: minls
	@echo Testing hello...
	@./minls TestImage
	@echo done.
