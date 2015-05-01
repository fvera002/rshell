FLAGS = -g -Wall -Werror -ansi -pedantic
BIN = bin

all: rshell ls cp

rshell: src/cmd.h 
	mkdir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell
	
ls:
	mkdir -p $(BIN)
	cd bin; g++ ../src/ls.cpp $(FLAGS) -o ls
	
cp: Timer.h
	mkdir -p $(BIN)
	cd bin; g++ ../src/cp.cpp $(FLAGS) -o cp

clean: 
	rm -rf bin
