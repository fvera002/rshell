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
// ls -l | head -3 | tail -1 


// g++ -g -Wall -Werror -ansi -pedantic main.cpp

bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);

void exec2(cmd c)
{
    char **argv = c.toArray();  
    if(-1 == execvp(*argv, argv)){
        perror("There was an error in execvp()");
    }
}

void piping(vector <cmd> & v) {
    int savedIn = dup(0);
    if (savedIn == -1)
        perror("There was an error in dup");
    int savedOut = dup(0);
    if (savedOut == -1)
        perror("There was an error in dup");
    
    //string top = c.front();
    
    int in ;
    in = 0;

    int output;
    output = 1;
    
    int fd[2];
    size_t i;
    for (i = 0; i < v.size() - 1; ++i) {
        if (pipe(fd) == -1)
            perror("There was an error in pipe");
        size_t pid = fork();
        size_t q = -1;
        if (pid == q)
            perror("There was an error in fork");
        if (pid == 0) {
            if ( in != 0) {
                if (dup2( in , 0) == -1) {
                    perror("There was an error in dup2 1");
                    return;
                }
                if (close( in ) == -1)
                    perror("There was an error in close");
            }
            if (dup2(fd[1], 1) == -1)
                perror("There was an error in dup2");
            if (close(fd[1]) == -1)
                perror("There was an error in close");
            exec2(v.at(i));
            exit(1);
        } else {
            if (close(fd[1]) == -1)
                perror("There was an error in close"); in = fd[0];
        }
    }
    if (dup2( in , 0) == -1)
        perror("There was an error in dup2 2");
    size_t pid = fork();
    size_t q = -1;
    if (pid == q)
        perror("There was an error in fork");
    if (pid == 0) {
        if (output != STDOUT_FILENO) {
            if (dup2(output, 1) == -1)
                perror("There was an error in dup2 3");
            if (close(output) == -1)
                perror("There was an error in closing fd");
        } else {
            if (dup2(savedOut, STDOUT_FILENO) == -1)
                perror("There was an error in dup2");
        }
        exec2(v.at(v.size() - 1));
    } else if (pid > 0){
        if (waitpid(pid, NULL, 0) == -1)
            perror("There was an error in wait");
    }
    if (dup2(savedOut, 1) == -1)
        perror("There was an error in dup2");
    if (dup2(savedIn, 0) == -1)
        perror("There was an error in dup2");
    if (close(savedOut) == -1)
        perror("There was an error in close");
    if (close(savedIn) == -1)
        perror("There was an error in close");
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

bool pipesPrep(queue<cmd> &commands, queue<string> &connectors)
{
    vector<cmd> pipes;
    //PUSH first 2 commands
    if(!commands.empty()) pipes.push_back(commands.front());
    if(!connectors.empty()) connectors.pop();
    if(!commands.empty()) commands.pop();
    
    if(!commands.empty()) pipes.push_back(commands.front());
    if(!commands.empty()) commands.pop();
        
    while( !connectors.empty() && !commands.empty() && connectors.front() == "|"){
        
        if(!commands.empty()) pipes.push_back(commands.front());
        if(!connectors.empty()) connectors.pop();
        if(!commands.empty()) commands.pop();
    }
    cout << pipes.size() <<endl;
    FOR(pipes){
        cout << pipes[i].toString() <<endl;
    }
    piping(pipes);
    return true;
    
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
    bool ok;
    if(!con.empty() && isRedirect(con) ){
        ok = redirectPrep(commands, connectors);
    } 
    else if(!con.empty() && con == "|" ){
        ok = pipesPrep(commands, connectors);
        if(commands.empty()) return;
    } 
    
    
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
        if(commands.empty()) return;
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
        cout << flush;
        cin.clear();
    }
    return 0;
}
