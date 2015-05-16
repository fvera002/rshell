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
// echo aaa> a; echo bbb > b&& echo ccc> chead -
// ls -l | head -3 | tail -1 
// ls -l | head -3 | tail -1 > lal
// ls -l | head -3 | tail -1 >> lal
// echo a && echo b || echo c > out


//TO DO
// ./a.out < filea12 > t123
// echo a && echo b || echo c > out
// ls -l | head -3 | tail -1 >> oiu ; echo aaa
// ./a.out < filea12 | tr A-Z a-z | tee newOutputFile1 | tr a-z A-Z > newOutputFile2


// g++ -g -Wall -Werror -ansi -pedantic main.cpp
// valgrind --tool=memcheck --leak-check=full ./t

bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);
vector<cmd> pipesPrep(queue<cmd> &commands, queue<string> &connectors);

bool exec2(cmd c)
{
    char **argv = c.toArray();  
    if(-1 == execvp(*argv, argv)){
        cout << c.toString() << ": ";
        perror(string(c.toString() + ": There was an error in execvp()").c_str());
    }
    return true;
}

bool piping(vector <cmd> &v, const char * ff1, const char * ff2, int flags1, int flags2) {
    //cout  <<  "t1 inico : "<< ff << f_in <<endl;
    /*
    cout << 11111111 <<endl;
    FOR(v){
        cout<< v[i].toString() <<endl;
    }
    cout << 222222 <<endl;
    */
    
    int savedIn = dup(0);
    if (savedIn == -1)
        perror("There was an error in dup");
    int savedOut = dup(1);
    if (savedOut == -1)
        perror("There was an error in dup");

    int in = 0;
    int output = 1;
    
    if(ff1 != NULL){
        //cout << 6666666 << ff1 << endl;
        in = open(ff1, flags1, S_IRUSR | S_IWUSR);
        if (in == -1) {
            perror("There was an error with open()");
            //exit(1);
        }
    }
    if(ff2 != NULL ){
        //cout << 7777 << ff2 << endl;
        output = open(ff2, flags2, S_IRUSR|S_IWUSR);
        if (output == -1){
            perror("There was an error with open()");
            //exit(1);
        }
    }
    
    int fd[2];
    
    //initialize all children expect the last one
    for (unsigned i = 0; i < v.size() - 1; ++i) {
        if (pipe(fd) == -1)
            perror("There was an error in pipe");
            
        int pid = fork();
        if (pid == -1)
            perror("There was an error in fork");
        if (pid == 0) {
            if (in != 0) {
                if (dup2(in , 0) == -1) {
                    perror("There was an error in dup2 1");
                    return false;
                }
                //if (close( in ) == -1)
                //    perror("There was an error in close");
            }
            if (dup2(fd[1], 1) == -1)
                perror("There was an error in dup2");
        
            if (close(fd[1]) == -1)
                perror("There was an error in close");
 
            exec2(v.at(i));
            
            exit(1); //avoid zoombies
        } else {
            if (close(fd[1]) == -1)
                perror("There was an error in close"); in = fd[0];
        }
    }
    if (dup2( in , 0) == -1)
        perror("There was an error in dup2 2");
        
    //now take care of last command
    int pid = fork();
    int q = -1;
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
        
    return true;
}

bool execRedirect(cmd currCmd, const char * ff, int flags, int fd, vector<cmd> pipes)
{
    // 0 = cin
    // 1 = cout
    // 2 = cerr
    
    int savedIn = dup(0);
    if (savedIn == -1)
        perror("There was an error in dup");
    int savedOut = dup(1);
    if (savedOut == -1)
        perror("There was an error in dup");
        
    int fl;
    int old = dup(fd);
    bool ret =false;
    
    if(old == -1){
        perror("There was an error with dup()");
        //exit(1);
        return false;
    }
    
    if(close(fd) == -1){
        perror("There was an error with close()");
        //exit(1);
        return false;
    }
    
    fl = open(ff, flags, S_IRUSR|S_IWUSR);
    if (fl == -1){
        perror("There was an error with open()");
        //exit(1);
    }
    else {
        //from now on everything is going to be printed into the file
        if(pipes.empty())
            ret = exec(currCmd);
        
        if(close(fl) == -1){
            perror("There was an error with close(). ");
            //exit(1);
        }
    }
    if(dup2(old, fd) == -1){
        perror("There was an error with dup2()");
        //exit(1);
    }
    

    return ret;
    
}

bool redirect(queue<cmd> &commands, queue<string> &connectors, int flags, int fd, vector<cmd> pipes)
{
    //if there's no file passed in, do nothing  
    cmd currCmd;  
    if(pipes.empty()){
        if(commands.size() < 2) return false; 
        currCmd = commands.front();
        commands.pop();
    }

    string file_name = commands.front().toString();
    //cout << "Printing into: " << file_name << endl;
    bool ret =false;
    if(!pipes.empty()) {
        piping(pipes, NULL, file_name.c_str(), 0, flags);
        ret = true;        
    }
    else {
        ret = execRedirect(currCmd, file_name.c_str(), flags, fd, pipes);
    }
    
    if(!commands.empty())commands.pop();
    if(!connectors.empty())connectors.pop();
    return ret;
}

bool redirectPrep(queue<cmd> &commands, queue<string> &connectors, vector<cmd> pipes)
{
    string con;
    if(!connectors.empty()) con = connectors.front();
    //cout << "--" << con <<endl;
    
    const int trunc = O_CREAT|O_WRONLY|O_TRUNC;
    const int append = O_CREAT|O_WRONLY|O_APPEND;
    const int read = O_RDONLY;
    
    if(con == ">" || con == "1>")
        return redirect(commands, connectors, trunc, 1, pipes); //TRUNC
    else if(con == ">>" || con == "1>>")
        return redirect(commands, connectors, append,  1, pipes); //APPEND
    else if(con == "2>")
        return redirect(commands, connectors, trunc,  2, pipes); //TRUNC
    else if(con == "2>>")
        return redirect(commands, connectors, append,  2, pipes);//APPEND
    else if(con == "<")
        return  redirect(commands, connectors, read,  0, pipes);//READ
    
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
            perror(string(c.toString() + ": There was an error in execvp()").c_str());
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

vector<cmd> pipesPrep(queue<cmd> &commands, queue<string> &connectors)
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
    
    //piping(pipes);
    return pipes;
    
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
    
    vector<cmd> pipes;
    if(!con.empty() && con == "|" ){
        pipes = pipesPrep(commands, connectors); 
        if(!connectors.empty()) con = connectors.front();       
    } 
    if(!con.empty() && isRedirect(con) ){
        ok = redirectPrep(commands, connectors, pipes);
    } 
    if(commands.empty()) return;
    
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
    vector<cmd> pipes;
    //if the next connector in queue is a redirect output
    if(!con.empty() && con == "|" ){
        pipes = pipesPrep(commands, connectors);
         if(!connectors.empty()) con = connectors.front(); 
    } 
    if(!con.empty() && isRedirect(con) ){
        ok = redirectPrep(commands, connectors, pipes);
    } 
    else if(!pipes.empty()) ok = piping(pipes, NULL, NULL, 0, 0);
    else {
        ok = exec(com);
        commands.pop();
    }
    if(commands.empty()) return;
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

