PROG = buildCollection
CC = g++
FLAGS = -g -Wall -std=c++11
OGJS = index.o dictionary.o buildCollection.o

$(PROG): $(OGJS)
	$(CC) $(FLAGS) -o $(PROG) $(OGJS)

buildCollection.o: index.hpp dictionary.hpp
	$(CC) $(FLAGS) -c buildCollection.cpp

index.o: index.hpp
	$(CC) $(FLAGS) -c index.cpp

dictionary.o: dictionary.hpp
	$(CC) $(FLAGS) -c dictionary.cpp

.PHONY: clean
clean:
	rm -rf *.o