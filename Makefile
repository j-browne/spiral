all: spiral

spiral: main.o
	gcc -o spiral -lpng main.o

main.o: main.c
	gcc -c -g main.c
