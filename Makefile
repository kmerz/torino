HEADERS = 

default: torino

main.o: main.c $(HEADERS)
	    gcc -c main.c -o main.o

torino: main.o
	    gcc -std=c99 -Wall main.o -o torino -l readline

clean:
	    -rm -f main.o
	        -rm -f torino
