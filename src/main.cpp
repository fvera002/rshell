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

#define FOR(x) for (unsigned i =0 ; i < (x).size(); ++i) 

//echo a || echo b && echo c || echo d
//echo a || echo b && echo c && echo d
// echo aaa > a; echo bbb > b && echo ccc > c
// echo aaa> a; echo bbb > b&& echo ccc> c
// echo aaa 2>> a is printing 2


// g++ -g -Wall -Werror -ansi -pedantic main.cpp

bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);


bool redirect(queue<cmd> &commands, queue<string> &connectors, bool trunc, int fd)
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
    
    if(old == -1){
        perror("There was an error with dup()");
        exit(1);
    }
    
    if(close(1) == -1){
        perror("There was an error with close()");
        exit(1);
    }
    
    if(trunc)
        fl = open(file_name, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    else 
        fl = open(file_name, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR);

    if (fl == -1){
        perror("There was an error with open()");
        exit(1);
    }
    //from now on everything is going to be printed into the file
    //cout << "_"<< currCmd.toString() << "_"<< endl;
    bool ret = exec(currCmd);
    
    if(close(fl) == -1){
        perror("There was an error with close(). ");
        exit(1);
    }
    
    if(dup2(old, 1) == -1){
        perror("There was an error with dup2()");
        exit(1);
    }
    
    //if(!commands.empty())commands.pop();
    if(!connectors.empty())connectors.pop();
    return ret;
}

bool redirectPrep(queue<cmd> &commands, queue<string> &connectors)
{
    string con;
    if(!connectors.empty()) con = connectors.front();
    
    if(con == ">" || con == "1>")
        return redirect(commands, connectors, true, 1); //TRUNC
    else if(con == ">>" || con == "1>>")
        return redirect(commands, connectors, false,  1); //APPEND
    else if(con == "2>")
        return redirect(commands, connectors, true,  2); //TRUNC
    else if(con == "2>>")
        return redirect(commands, connectors, false,  2);//APPEND
    else if(con == "<")
        return redirect(commands, connectors, false,  0);//TRUNC
    
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
