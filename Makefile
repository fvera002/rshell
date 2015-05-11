FLAGS = -g -Wall -Werror -ansi -pedantic
BIN = bin

all: rshell ls

rshell: src/cmd.h 
	mkdir -p $(BIN)
	cd bin; g++ ../src/main.cpp $(FLAGS) -o rshell
	
ls:
	mkdir -p $(BIN)
	cd bin; g++ ../src/ls.cpp $(FLAGS) -o ls
	
cp: src/Timer.h
	mkdir -p $(BIN)
	cd bin; g++ ../src/cp.cpp $(FLAGS) -o cp

rm: 
	mkdir -p $(BIN)
	cd bin; g++ ../src/rm.cpp $(FLAGS) -o rm
	
mv: 
	mkdir -p $(BIN)
	cd bin; g++ ../src/rm.cpp $(FLAGS) -o rm
	
clean: 
	rm -rf bin
