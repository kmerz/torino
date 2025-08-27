CC = gcc
objects = main.o mpc.o eval.o

default: torino

$(objects): %.o: %.c 
	    $(CC) -g -c $^ -o $@

all: $(objects)
	    $(CC) -std=c99 -Wall $(objects) -o torino -l readline

clean:
	    -rm -f main.o
	        -rm -f torino
