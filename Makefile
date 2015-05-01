FLAGS = -g -Wall -Werror -ansi -pedantic
BIN = bin

all:
	rshell

rshell: src/cmd.h 
	mkdir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell
	
ls:
	mkdir -p $(BIN)
	cd bin; g++ ../src/ls.cpp $(FLAGS) -o ls

clean: 
	rm -rf bin
