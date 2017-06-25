make : main.o file_read.o simulator.o cache.o data_structures.o Iterator.o
	gcc -g -lm -o simulator main.o file_read.o simulator.o cache.o data_structures.o Iterator.o

main.o : main.c
	gcc -g -c main.c

file_read.o : file_read.c
	gcc -g -c file_read.c

simulator.o : simulator.c
	gcc -g -c simulator.c

Iterator.o : Iterator.c
	gcc -g -c Iterator.c

cache.o : cache.c
	gcc -g -c cache.c

data_structures.o : data_structures.c
	gcc -g -c data_structures.c

clean :
	rm *.o simulator
