.PHONY: all clean

INCLUDE = -Iinclude -Iinclude/igraph
LDPARAMS = -Llib
LIBS = -ligraph -lsc
BIN = bin
SRC = src
LIB = lib

all: libs kruskal louvain girvan_newman influence_det_spanning_graph

libs:
	$(CC) -c $(INCLUDE) -fPIC $(SRC)/libsc.c -o $(BIN)/libsc.o
	$(CC) -shared -o $(LIB)/libsc.so $(BIN)/libsc.o

kruskal:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/kruskal_community.c -o $(BIN)/kruskal.o $(LIBS)

louvain:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/louvain.c -o $(BIN)/louvain.o $(LIBS)

girvan_newman:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/girvan_newman.c -o $(BIN)/girvan_newman.o $(LIBS)

influence_det_spanning_graph:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/influence_det_spanning_graph.c -o $(BIN)/influence_det_spanning_graph.o $(LIBS)
