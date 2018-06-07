#http://blog.jgc.org/2011/07/gnu-make-recursive-wildcard-function.html
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC=g++
CFLAGS+=-I. -Wall -std=c++14
LIBS = -lcpp_redis -ltacopie -lpthread
SRC = $(call rwildcard,, *.cpp)
OBJ = $(SRC:.cpp=.o)
DEPS = $(SRC:.cpp=.d)
DEBUG = -g
DEBUG-OPTIMIZE = -g -O3
OPTIMIZE = -O3
PROF = -pg -O3

index: $(OBJ)
	$(CC) -o ./exe/$@ $^ $(CFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) -MMD -MP -c -o $@ $< $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.d' -delete

.PHONY: debug
debug: CFLAGS+=$(DEBUG)
debug: index

.PHONY: optimize
optimize: CFLAGS+=$(OPTIMIZE)
optimize: index

.PHONY: prof
prof: CFLAGS+=$(PROF)
prof: index

-include $(DEPS)