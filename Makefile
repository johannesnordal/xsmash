CC=g++
CDEPS=-lz
CFLAGS=-Wall -Ofast -std=c++11
BIN=bin
SRC=src
LB=src/bifrost/src

all: $(BIN) $(BIN)/sketch $(BIN)/dist $(BIN)/cluster

$(BIN):
	mkdir $@

$(BIN)/sketch: $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp
	$(CC) $(CFLAGS) $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp \
		-o $(BIN)/sketch $(CDEPS)

$(BIN)/dist: $(SRC)/Dist.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Dist.cpp -o $(BIN)/dist

$(BIN)/cluster: $(SRC)/Cluster.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Cluster.cpp -o $(BIN)/cluster

clean:
	rm -rf $(BIN)
	rm -f *.sketch fastx/*.xsketch fastx/*.msh fastx/*.sketch
