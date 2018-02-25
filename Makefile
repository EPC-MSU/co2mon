CC=gcc
CFLAGS=-std=c99

all: co2mon.o main.o server.o ringbuf.o
	mkdir bin
	$(CC) -o bin/co2mond co2mon.o main.o server.o ringbuf.o -Wl,-rpath=/usr/local/lib -L /usr/local/lib -lhidapi-hidraw -lmicrohttpd

main.o: co2mond/src/main.c
	$(CC) -c -o main.o co2mond/src/main.c $(CFLAGS)

server.o: co2mond/src/server.c
	$(CC) -c -o server.o co2mond/src/server.c $(CFLAGS)

ringbuf.o: ringbuf/src/ringbuf.c
	$(CC) -c -o ringbuf.o ringbuf/src/ringbuf.c $(CFLAGS)

co2mon.o: libco2mon/src/co2mon.c
	$(CC) -c -o co2mon.o libco2mon/src/co2mon.c $(CFLAGS)

clean:
	rm -f ./*.o
	rm -R bin
