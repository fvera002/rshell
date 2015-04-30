FLAGS = -g -Wall -Werror -ansi -pedantic
BIN = bin

all: src/main.cpp src/cmd.h
	mkdir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell

rshell: src/cmd.h 
	mdkir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell
	
cp: src/cp.cpp
	mdkir -p $(BIN)
	cd bin; g++ ../src/cp.cpp $(FLAGS) -o cp
	
clean: 
	cd bin; if [ -a rshell ] ; then rm rshell; fi
	rmdir bin	
