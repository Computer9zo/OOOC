make : main.o file_read.o simulator.o cache.o data_structures.o
	gcc -lm -g -o simulator main.o file_read.o simulator.o cache.o data_structures.o
main.o : main.c
	gcc -c main.c

file_read.o : file_read.c
	gcc -c file_read.c

simulator.o : simulator.c
	gcc -c simulator.c

cache.o : cache.c
	gcc -c cache.c

data_structures.o : data_structures.c
	gcc -c data_structures.c

clean :
	rm *.o simulator
