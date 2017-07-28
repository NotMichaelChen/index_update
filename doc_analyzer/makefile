CC=g++
CFLAGS+=-I. -Wall -std=c++14
VPATH = /Matcher
DEPS = $(wildcard *.h)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
DEBUG = -g -O0

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

LBS: $(OBJ)
	$(CC) -o ./exe/$@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o

.PHONY: debug
debug: CFLAGS+=$(DEBUG)
debug: LBS