CC = gcc
HEADERS = mpc.h
objects = main.o mpc.o

default: torino

$(objects): %.o: %.c 
	    $(CC) -g -c $^ -o $@

all: $(objects)
	    $(CC) -std=c99 -Wall $(objects) -o torino -l readline

clean:
	    -rm -f main.o
	        -rm -f torino
