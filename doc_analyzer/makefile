CC=g++
CFLAGS=-I. -Wall -std=c++14
DEPS = $(wildcard *.h)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

LBS: $(OBJ)
	$(CC) -o ./exe/$@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o

.PHONY: debug
debug: $(OBJ)
	$(CC) -o ./exe/$@ $^ $(CFLAGS) -g -O0