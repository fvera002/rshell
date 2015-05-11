#include <iostream>     // cin, cout
#include <fstream>      // ifstream
#include <stdlib.h>      // ifstream
#include <stdio.h>      // ifstream
#include "Timer.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

#define CALC_TIME(x,y) { \
    t.start(); \
    (x)(argv[1], argv[2]);\
    cout << (y)<< endl;\
    t.elapsedWallclockTime(etime);\
    cout << "wallclock time: " << etime << endl;\
    t.elapsedUserTime(etime);\
    cout << "user time: " << etime << endl;\
    t.elapsedSystemTime(etime);\
    cout << "system time: " << etime << endl << endl;\
}

bool exist(char * f)
{
    struct stat st;
    if(stat(f, &st) == 0) {
        return true;
    }
    return false;
}

void exist_error(char * f)
{
    struct stat st;
    if(stat(f, &st) == 0) {
        if(S_ISDIR(st.st_mode)){
            cerr << "ERROR: Destination \"" << f << "\" is a directory."<< endl; 
        }
        else{
            cerr << "ERROR: Output file named \"" << f << "\" already exists." << endl;            
        }
        cerr << "Please, enter an non-existent output file as parameter." <<endl <<endl;
        exit(1);
    } 
    else {
        perror("ERROR: There was an error with stat(). ");
        exit(1);
    }
}

//ofstream and iostream
void copy1(char * from, char *to)
{

    ifstream is(from);          // open file
    ofstream outfile (to);
    
    char c;
    while (is.get(c)){          // loop getting single characters
        //cout << c;
        outfile.put(c);
    }

    is.close();                // close file
    
}

//read() and write() one character at a time
void copy2(char *from, char *to)
{
    int fdnew;
    int fdold;
    if (-1 == (fdnew = open(to, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR))) {
        perror("ERROR: There was an error with open(). ");
        exit(1);
    }
    if (-1 == (fdold = open(from, O_RDONLY))) {
        perror("ERROR: There was an error with open(). ");
        exit(1);
    }
    int size;
    char c[1];
    
    if (-1 == (size = read(fdold, c, sizeof(c)))) {
        perror("ERROR: There was an error with read(). ");
        exit(1);
    }
    
    while (size > 0) {
        if (-1 == write(fdnew, c, size)) {
            perror("ERROR: There was an error with write(). ");
            exit(1);
        }
        if (-1 == (size = read(fdold, c, sizeof(c)))) {
            perror("ERROR: There was an error with read(). ");
            exit(1);
        }
    }
    if (-1 == close(fdnew)) {
        perror("ERROR: There was an error with close(). ");
        exit(1);
    }
    if (-1 == close(fdold)) {
        perror("ERROR: There was an error with close(). ");
        exit(1);
    }

    
}

//read() and write() buffer size
void copy3(char *from, char *to)
{
    int fdnew;
    int fdold;
    if (-1 == (fdnew = open(to, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR))) {
        perror("ERROR: There was an error with open(). ");
        exit(1);
    }
    if (-1 == (fdold = open(from, O_RDONLY))) {
        perror("ERROR: There was an error with open(). ");
        exit(1);
    }
    int size;
    char c[BUFSIZ];
    
    if (-1 == (size = read(fdold, c, sizeof(c)))) {
        perror("ERROR: There was an error with read(). ");
        exit(1);
    }
    
    while (size > 0) {
        if (-1 == write(fdnew, c, size)) {
            perror("ERROR: There was an error with write(). ");
            exit(1);
        }
        if (-1 == (size = read(fdold, c, sizeof(c)))) {
            perror("ERROR: There was an error with read(). ");
            exit(1);
        }
    }
    if (-1 == close(fdnew)) {
        perror("ERROR: There was an error with close(). ");
        exit(1);
    }
    if (-1 == close(fdold)) {
        perror("ERROR: There was an error with close(). ");
        exit(1);
    }
    
}

int main(int argc, char** argv)
{
    //Your final program should have two required arguments
    if(argc < 3){
        cerr << "ERROR: Input and/or output file is missing." <<endl <<endl;
        exit(1);
    }
    if( ! exist(argv[1])){
        cerr << "ERROR: Input file does not exist." <<endl <<endl;
        exit(1);
    }
    exist_error(argv[2]);
    
    if(argc == 3){
        //most efficient one
        copy3(argv[1], argv[2]);
    } 
    else {
        //run read and write via character
        Timer t;
        double etime;
        CALC_TIME(copy1, "1- Using ofstream and iostream one character at a time");
        CALC_TIME(copy2, "2- Using read() and write() one character at a time");
        CALC_TIME(copy3, "3- Using read() and write() and BUFSIZ");
    }
    
    return 0;
}
