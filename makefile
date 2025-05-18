all: simplefs

simplefs: main.o fs.o
	gcc -o simplefs main.o fs.o

main.o: main.c fs.h
	gcc -c main.c

fs.o: fs.c fs.h
	gcc -c fs.c

clean:
	rm -f *.o simplefs
