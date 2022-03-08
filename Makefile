CC=g++
CFLAGS=-Wall -Ofast -std=c++11
BIN=bin
SRC=src
CDEPS=lib/libbifrost.a -lz -pthread

all: $(BIN) $(BIN)/sketch $(BIN)/dist $(BIN)/cluster

$(BIN)/sketch: $(SRC)/Sketch.cpp
	$(CC) $(CFLAGS) $(SRC)/Sketch.cpp -o $(BIN)/sketch $(CDEPS)

$(BIN)/dist: $(SRC)/Dist.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Dist.cpp -o $(BIN)/dist

$(BIN)/cluster: $(SRC)/Cluster.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Cluster.cpp -o $(BIN)/cluster

clean:
	rm -rf $(BIN)/sketch
	rm -f *.sketch fastx/*.xsketch fastx/*.msh fastx/*.sketch
