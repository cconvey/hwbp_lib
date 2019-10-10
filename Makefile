all: libhwbp.a libhwbp.so libhwbp_null.so test

hwbp.o: hwbp.c hwbp.h
	gcc -fPIC -std=c99 -g -O0 -c -o hwbp.o hwbp.c

hwbp_null.o: hwbp_null.c hwbp.h
	gcc -fPIC -std=c99 -g -O0 -c -o hwbp_null.o hwbp_null.c

libhwbp.a: hwbp.o
	ar crs libhwbp.a hwbp.o

libhwbp.so: hwbp.o
	gcc -shared hwbp.o -o libhwbp.so

libhwbp_null.so: hwbp_null.o
	gcc -shared hwbp_null.o -o libhwbp_null.so

test: libhwbp.a hwbp.h test.cpp
	g++ -o test --std=c++11 -g -O0 test.cpp libhwbp.a

clean:
	rm -f hwbp.o libhwbp.a libhwbp.so libhwbp_null.so test

