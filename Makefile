CC=g++
CDEPS=-lz
CFLAGS=-Ofast -std=c++11
BIN=bin
SRC=src
LB=src/bifrost/src

all: $(BIN) $(BIN)/sketch

$(BIN):
	mkdir $@

$(BIN)/sketch: $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp \
	$(SRC)/CandidateSet.hpp
	$(CC) $(CFLAGS) $(SRC)/Sketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp \
		-o $(BIN)/sketch $(CDEPS)

clean:
	rm -rf $(BIN)
	rm -f *.sketch fastx/*.xsketch fastx/*.msh fastx/*.sketch
