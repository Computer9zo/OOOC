simulator: main.o simulator.o file_read.o data_structures.o 
	gcc -std=c99 -o simulator main.o simulator.o file_read.o data_structures.o

main.o: main.c
	gcc -c -v -std=c99 main.c

simulator.o: simulator.c
	gcc -c -v -std=c99 simulator.c

file_read.o: file_read.c
	gcc -c -v -std=c99 file_read.c

data_structures.o: data_structures.c
	gcc -c -v -std=c99 data_structures.c

all: simulator

clean: rm -rf *.o simulator
