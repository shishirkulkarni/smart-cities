.PHONY: all clean

INCLUDE = -Iinclude -Iinclude/igraph
LDPARAMS = -Llib
LIBS = -ligraph
BIN = bin
SRC = src

all: kruskal

kruskal:
	$(CC) $(INCLUDE) $(LDPARAMS) $(SRC)/kruskal_community.c -o $(BIN)/kruskal.o $(LIBS)

