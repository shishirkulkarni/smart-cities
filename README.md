# Smart Cities

A project implemented using igraph C library to analyze and compare different community and influence detection algrithms.

## Steps to set up the project
* `sudo apt-get install bison`
* `sudo apt-get install flex`
* `git submodule --init && git submodule --update`
* `cd deps/igraph`
* `./bootstrap.sh`
* `./configure`
* `make`
* `cp include/* ../../include`
* `cp .src/libs/libigraph.so* ../../lib`
* `cd ../..`
* `export LD_LIBRARY_PATH = lib/`
* `make`

You will find all the executables in the bin directory