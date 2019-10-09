all: libhwbp.a test

hwbp.o: hwbp.c hwbp.h
	gcc -std=c99 -g -O0 -c -o hwbp.o hwbp.c

libhwbp.a: hwbp.o
	ar crs libhwbp.a hwbp.o

test: libhwbp.a hwbp.h test.cpp
	g++ -o test --std=c++11 -g -O0 test.cpp libhwbp.a

clean:
	rm -f hwbp.o libhwbp.a test

