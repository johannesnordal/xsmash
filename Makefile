CC=g++
CFLAGS=-Ofast -std=c++11
BIN=local/bin
SRC=src
CDEPS=local/lib/libbifrost.a -lz -pthread

all: $(BIN) $(BIN)/sketch $(BIN)/dist $(BIN)/cluster $(BIN)/mince_server \
	$(BIN)/mince_client

$(BIN)/sketch: $(SRC)/Sketch.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Sketch.cpp -o $(BIN)/sketch $(CDEPS)

$(BIN)/dist: $(SRC)/Dist.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Dist.cpp -o $(BIN)/dist $(CDEPS)

$(BIN)/cluster: $(SRC)/Cluster.cpp $(SRC)/Sketch.hpp
	$(CC) $(CFLAGS) $(SRC)/Cluster.cpp -o $(BIN)/cluster $(CDEPS)

$(BIN)/mince_server: $(SRC)/MinceServer.cpp $(SRC)/Triangulate.hpp \
	$(SRC)/ServerCommon.hpp
	$(CC) $(CFLAGS) $(SRC)/MinceServer.cpp -o $(BIN)/mince_server

$(BIN)/mince_client: $(SRC)/MinceClient.cpp $(SRC)/Sketch.hpp \
	$(SRC)/Triangulate.hpp $(SRC)/ServerCommon.hpp
	$(CC) $(CFLAGS) $(SRC)/MinceClient.cpp -o $(BIN)/mince_client $(CDEPS)
clean:
	rm -rf $(BIN)/sketch $(BIN)/cluster $(BIN)/dist $(BIN)/mince_server \
		$(BIN)/mince_client
	rm -f *.sketch fastx/*.xsketch fastx/*.msh fastx/*.sketch
