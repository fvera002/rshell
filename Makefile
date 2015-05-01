FLAGS = -g -Wall -Werror -ansi -pedantic
BIN = bin

all: src/main.cpp src/cmd.h
	mkdir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell

rshell: src/cmd.h 
	mdkir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell
	
ls:
	mdkir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o ls

clean: 
	rm -rf bin	
