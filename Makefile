CC=g++
CDEPS=-lz
CFLAGS=-Ofast -std=c++11
BIN=bin
SRC=src
LB=src/bifrost/src

all: $(BIN) $(BIN)/sketch $(BIN)/dist

$(BIN):
	mkdir $@

$(BIN)/sketch: $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp
	$(CC) $(CFLAGS) $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp \
		-o $(BIN)/sketch $(CDEPS)

$(BIN)/dist: $(SRC)/Dist.cpp $(SRC)/Utils.hpp
	$(CC) $(CFLAGS) $(SRC)/Dist.cpp -o $(BIN)/dist

clean:
	rm -rf $(BIN)
	rm -f *.sketch fastx/*.xsketch fastx/*.msh fastx/*.sketch
