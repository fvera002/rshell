#include "cmd.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <queue>

#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

//echo a || echo b && echo c || echo d
//echo a || echo b && echo c && echo d
// echo aaa > a; echo bbb > b && echo ccc > c
// echo aaa> a; echo bbb > b&& echo ccc> c
// echo aaa 2>> a is printing 2


// g++ -g -Wall -Werror -ansi -pedantic main.cpp

bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);

void piping(vector<cmd> & v) {
    const int PIPE_READ = 0;
    const int PIPE_WRITE = 1;
    int fd[2];
    if(pipe(fd) == -1)//call to pipe, it puts the read end and write end file descriptors in fd
       perror("There was an error with pipe(). ");
    
    int pid = fork();
    if(pid == -1)//fork’s return value for an error is -1
    {
       perror("There was an error with fork(). ");
       exit(1);//there was an error with fork so exit the program and go back and fix it
    }
    else if(pid == 0)//when pid is 0 you are in the first child process
    {
       cout<<"This is the first child process ";
    
       //write to the pipe
       if(-1 == dup2(fd[PIPE_WRITE],1))//make stdout the write end of the pipe
          perror("There was an error with dup2. ");
       if(-1 == close(fd[PIPE_READ]))//close the read end of the pipe because we're not doing anything with it right now
          perror("There was an error with close. ");
    
       char ** argv = v.at(0).toArray();
       if(-1 == execvp(argv[0], argv))
          perror("There was an error in execvp. ");
    
    
       exit(1);  //prevents zombie process
    }
    //if pid is not 0 then we’re in the first parent
    else if(pid > 0) //first parent function
    {
       //read end of the pipe
       int savestdin;
       if(-1 == (savestdin = dup(0)))//need to restore later or infinite loop
          perror("There is an error with dup. ");
       if( -1 == wait(0)) //wait for the child process to finish executing
          perror("There was an error with wait(). ");
    
        
       int pid2 = fork();
       if(pid2 == -1)//fork's return value for an error is -1
       {
          perror("There was an error with fork(). ");
          exit(1);//there was an error with fork so exit the program and go back and fix it
       }
       else if(pid2 == 0)//when pid2 is 0 you are in the second child process
       {
          cout << "This is the second child process ";
    
          if(-1 == dup2(fd[PIPE_READ],0))//make stdin the read end of the pipe
             perror("There was an error with dup2. ");
          if(-1 == close(fd[PIPE_WRITE])) //close the write end of the pipe because we're not doing anything with it right now
             perror("There was an error with close. ");
    
          char ** argv2 = v.at(1).toArray();
          if(-1 == execvp(argv2[0], argv2))
             perror("There was an error in excecvp. ");
    
          exit(1); //prevents zombie process
       }
       else if(pid2 > 0) //second parent function
       {
          if (-1 == close(fd[PIPE_WRITE])) //close the write end of the pipe in the parent so the second child isn't left waiting
             perror("There was an error with close. ");
          if(-1 == wait(0)) //wait for the child process to finish executing
             perror("There was an error with wait(). ");
       }
    
       if(-1 == dup2(savestdin,0))//restore stdin
          perror("There is an error with dup2. ");
    }

}

bool redirectIn(queue<cmd> &commands, queue<string> &connectors, int flags, int fd)
{
    //if there's no file passed in, do nothing    
    if(commands.size() < 2) return false; 
    
    cmd currCmd = commands.front();
    commands.pop();
    
    //char * file_name;
    char * file_name = commands.front().toArray()[0];
    //cout << "Printing into: " << file_name << endl;
    
    int status;
    int pid = fork();
    if(pid == -1){//fork’s return value for an error is -1
        perror("There was an error with fork()");
        exit(1);//there was an error with fork so exit the program and go back and fix it
    }
    else if(pid == 0){//when pid is 0 you are in the child process
        //This is the child process
        int fl;
        int old = dup(fd);
        bool ret =false;
        
        if(old == -1){
            perror("There was an error with dup()");
            //exit(1);
            if(!connectors.empty())connectors.pop();
            return ret;
        }
        
        if(close(fd) == -1){
            perror("There was an error with close()");
            //exit(1);
            if(!connectors.empty())connectors.pop();
            return ret;
        }
        
        fl = open(file_name, flags, S_IRUSR|S_IWUSR);
        if (fl == -1){
            perror("There was an error with open()");
            //exit(1);
        }
        
        //if(!commands.empty())commands.pop();
        if(!connectors.empty())connectors.pop(); 
        char **argv = currCmd.toArray();  
        if(-1 == execvp(*argv, argv)){
            perror("There was an error in execvp()");
        }
        if(close(fl) == -1){
                perror("There was an error with close(). ");
                //exit(1);
            }
        if(dup2(old, 1) == -1){
            perror("There was an error with dup2()");
            //exit(1);
        }
        exit(1);
    }
    //if pid is not 0 then we’re in the parent
    //parent process
    else{
        pid = wait(&status);
        if(pid == -1){
            perror("There was an error in wait()");
            exit(1);
        }
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) > 0)return false;
            return true;
        }
    }
    return false; 
    
}

bool redirect(queue<cmd> &commands, queue<string> &connectors, int flags, int fd)
{
    //if there's no file passed in, do nothing    
    if(commands.size() < 2) return false; 
    
    cmd currCmd = commands.front();
    commands.pop();

    char * file_name = commands.front().toArray()[0];
    //cout << "Printing into: " << file_name << endl;
    
    
    // 0 = cin
    // 1 = cout
    // 2 = cerr
    int fl;
    int old = dup(fd);
    bool ret =false;
    
    if(old == -1){
        perror("There was an error with dup()");
        //exit(1);
        if(!connectors.empty())connectors.pop();
        return ret;
    }
    
    if(close(fd) == -1){
        perror("There was an error with close()");
        //exit(1);
        if(!connectors.empty())connectors.pop();
        return ret;
    }
    
    fl = open(file_name, flags, S_IRUSR|S_IWUSR);
    if (fl == -1){
        perror("There was an error with open()");
        //exit(1);
    }
    else {
        //from now on everything is going to be printed into the file
        //cout << "_"<< currCmd.toString() << "_"<< endl;
        ret = exec(currCmd);
        
        if(close(fl) == -1){
            perror("There was an error with close(). ");
            //exit(1);
        }
    }
    if(dup2(old, 1) == -1){
        perror("There was an error with dup2()");
        //exit(1);
    }
    
    //if(!commands.empty())commands.pop();
    if(!connectors.empty())connectors.pop();
    return ret;
}

bool redirectPrep(queue<cmd> &commands, queue<string> &connectors)
{
    string con;
    if(!connectors.empty()) con = connectors.front();
    //cout << "--" << con <<endl;
    
    const int trunc = O_CREAT|O_WRONLY|O_TRUNC;
    const int append = O_CREAT|O_WRONLY|O_APPEND;
    const int read = O_CREAT|O_RDONLY;
    
    if(con == ">" || con == "1>")
        return redirect(commands, connectors, trunc, 1); //TRUNC
    else if(con == ">>" || con == "1>>")
        return redirect(commands, connectors, append,  1); //APPEND
    else if(con == "2>")
        return redirect(commands, connectors, trunc,  2); //TRUNC
    else if(con == "2>>")
        return redirect(commands, connectors, append,  2);//APPEND
    else if(con == "<")
        return redirectIn(commands, connectors, read,  0);
    
    return false;
}

// runs fork and execvp returning true if execvp succeed
bool exec(cmd c)
{
    //cout<< c.toString() << endl;
    int status;
    int pid = fork();
    if(pid == -1){//fork’s return value for an error is -1
        perror("There was an error with fork()");
        exit(1);//there was an error with fork so exit the program and go back and fix it
    }
    else if(pid == 0){//when pid is 0 you are in the child process
        //This is the child process 
        char **argv = c.toArray();  
        if(-1 == execvp(*argv, argv)){
            perror("There was an error in execvp()");
        }
        exit(1);
    }
    //if pid is not 0 then we’re in the parent
    //parent process
    else{
        pid = wait(&status);
        if(pid == -1){
            perror("There was an error in wait()");
            exit(1);
        }
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) > 0)return false;
            return true;
        }
    }
    return false; 
}

// aux run() by spliting the root command
void runPrep(cmd &c)
{
    queue<cmd> commands;
    queue<string> connectors;
    
    //split commands using connectors an put them onto a queue
    commands = c.split(connectors);
    
    run(commands, connectors);
}

bool isRedirect(string con)
{
    //cout << "--" << con <<endl;
    FOR(con){
        if(con[i] == '<') return true;
        if(con[i] == '>') return true;
    }
    return false;
}

//same logic, however checks before running the command
void run2(queue<cmd> &commands, queue<string> &connectors, bool &prev)
{
    if(commands.empty()) return;
    //use the first command "highest priority" 
    cmd com = commands.front();
    if(com.toString() == "exit") exit(0);
    
    string con;
    if(!connectors.empty()) con = connectors.front();
    //cout<<prev;
    //cout << "2_"<< com.toString() << "_"<< endl;
    //cout << "2-" << con << "-" << endl;
    
    if(!con.empty() && isRedirect(con) ){
        prev = redirectPrep(commands, connectors);
    } 
    
    bool ok;
    if(prev){
    //SUCESS
        if(con == "||" ){
            if(!connectors.empty()) connectors.pop();
            if(!commands.empty()) commands.pop();
            ok =false;
        }
        else if(con == "&&" || con == ";"){
            if(!connectors.empty())connectors.pop();
            ok = exec(com);
            if(!commands.empty()) commands.pop();
        }
    }
    else{
    //FAIL
        if(con == "&&"){
            if(!connectors.empty()) connectors.pop();
            if(!commands.empty()) commands.pop();
            ok =false;
        }
        else if(con == "||" || con == ";"){
            if(!connectors.empty()) connectors.pop();
            ok = exec(com);
            if(!commands.empty()) commands.pop();
        }
    }
    
    
    run2(commands, connectors, ok);
    return ;
}

bool pipesPrep(queue<cmd> &commands, queue<string> &connectors)
{
    vector<cmd> pipes;
    //PUSH first 2 commands
    if(!commands.empty()) pipes.push_back(commands.front());
    if(!connectors.empty()) connectors.pop();
    if(!commands.empty()) commands.pop();
    if(!commands.empty()) pipes.push_back(commands.front());
        
    while( !connectors.empty() && !commands.empty() && connectors.front() == "|"){
        
        if(!commands.empty()) pipes.push_back(commands.front());
        if(!connectors.empty()) connectors.pop();
        if(!commands.empty()) commands.pop();
    }
    
    piping(pipes);
    return true;
    
}

// do the logics of commands, executing execpv when needed 
// then call run again recursively
// or exits the program if it's the case
void run(queue<cmd> &commands, queue<string> &connectors)
{
    if(commands.empty()) return;
    //use the first command "highest priority" 
    cmd com = commands.front();
    if(com.toString() == "exit") exit(0);
    
    string con;
    if(!connectors.empty())con= connectors.front();
    
    //cout << "_"<< com.toString() << "_"<< endl;
    //cout << "-" << con << "-" << endl;
    
    bool ok;
    //if the next connector in queue is a redirect output
    if(!con.empty() && isRedirect(con) ){
        ok = redirectPrep(commands, connectors);
    } 
    else if(!con.empty() && con == "|" ){
        ok = pipesPrep(commands, connectors);
    } 
    else {
        ok = exec(com);
    }
    commands.pop();
    if(!connectors.empty())con= connectors.front();

    
    if(ok){ 
    //SUCESS
        if(con == "||" ){
            if(!connectors.empty())connectors.pop();
            if(!commands.empty())commands.pop();
            run2(commands, connectors, ok);
        }
        else if(con == "&&" || con == ";"){
            if(!connectors.empty())connectors.pop();
            run(commands, connectors);
        }
    } 
    else {
    //FAIL
        if(con == "&&"){ 
            if(!connectors.empty())connectors.pop();
            if(!commands.empty())commands.pop();
            run2(commands, connectors, ok);
        }
        else if(con == "||" || con == ";"){ 
            if(!connectors.empty())connectors.pop();
            run(commands, connectors);
        }
    }
    
    return ;
}

// returns a string with user and machine name
string getPrompt(){
    char machine[128];
    string user;
    string prompt;
    
    struct passwd *pw  = getpwuid(getuid());
    int host = gethostname(machine,128);
    
    if(pw != NULL && host != -1){
        user = pw->pw_name;
        prompt = user + "@" + string(machine)  + "$ ";
    } else{
        prompt = "$ ";
        if(pw == NULL)
            perror("There was an error in getpwuid()");
        if(host == -1)
            perror("There was an error in gethostname()");
    }
    return prompt; 

}

// main program
// get input until in a infinite loop
// within functions are responsible for exiting the program
int main()
{
    while(true){
        cout << getPrompt();
        string st;
        getline (cin,st);
        if(st ==  "exit") exit(0);
        if(st.empty() || st == "#") continue;
        cmd c(st);
        runPrep(c);
        
    }
    return 0;
}

