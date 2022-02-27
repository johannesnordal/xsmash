CC=g++
CDEPS=-lz
CFLAGS=-Ofast -std=c++11
BIN=bin
LB=src/bifrost/src

all: $(BIN) $(BIN)/xsketch

$(BIN):
	mkdir $@

$(BIN)/xsketch: src/xsketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp
	$(CC) $(CFLAGS) src/xsketch.cpp $(LB)/Kmer.cpp $(LB)/KmerIterator.cpp \
		-o $(BIN)/xsketch $(CDEPS)

clean:
	rm -rf $(BIN)
	rm -f *.sketch fastx/*.xsketch fastx/*.msh
