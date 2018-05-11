#http://blog.jgc.org/2011/07/gnu-make-recursive-wildcard-function.html
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC=g++
CFLAGS+=-I. -Wall -std=c++14
LIBS = -lcpp_redis -ltacopie -lpthread
VPATH = %.cpp %.o Matcher
DEPS = $(call rwildcard,, *.hpp)
SRC = $(call rwildcard,, *.cpp)
OBJ = $(SRC:.cpp=.o)
DEBUG = -g -O0 -pg
OPTIMIZE = -O3
DEBUG-OPTIMIZE = -pg -O3

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

index: $(OBJ)
	$(CC) -o ./exe/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	find . -type f -name '*.o' -delete

.PHONY: debug
debug: CFLAGS+=$(DEBUG)
debug: index

.PHONY: optimize
optimize: CFLAGS+=$(OPTIMIZE)
optimize: index

.PHONY: debug-optimize
debug-optimize: CFLAGS+=$(DEBUG-OPTIMIZE)
debug-optimize: index