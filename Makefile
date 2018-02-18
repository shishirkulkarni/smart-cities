.PHONY: all clean

INCLUDE = -Iinclude -Iinclude/igraph
LDPARAMS = -Llib
LIBS = -ligraph -lsc
BIN = bin
SRC = src
LIB = lib

all: libs kruskal

libs:
	$(CC) -c $(INCLUDE) -fPIC $(SRC)/libsc.c -o $(BIN)/libsc.o
	$(CC) -shared -o $(LIB)/libsc.so $(BIN)/libsc.o

kruskal:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/kruskal_community.c -o $(BIN)/kruskal.o $(LIBS)

