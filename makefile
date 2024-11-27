CC = g++
CFLAGS = -std=c++17 -pthread -Wall
OBJS = memfs.o

all: memfs benchmark

memfs: main.o $(OBJS)
	$(CC) $(CFLAGS) -o memfs main.o $(OBJS)

benchmark: benchmark.o $(OBJS)
	$(CC) $(CFLAGS) -o benchmark benchmark.o $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o memfs benchmark
