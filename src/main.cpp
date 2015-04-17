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
using namespace std;

bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);

// runs fork and execvp returning true if execvp succeed
bool exec(cmd c)
{
    int status;
    int pid = fork();
    if(pid == -1){//fork’s return value for an error is -1
        perror("There was an error with fork()");
        exit(1);//there was an error with fork so exit the program and go back and fix it
    }
    else if(pid == 0){//when pid is 0 you are in the child process
        //This is the child process 
        char **argv = c.toArray();  
        if(-1 == execvp(*argv, argv))
            perror("There was an error in execvp()");
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

// do the logics of commands, executing execpv when needed 
// then call run again recursively
// or exits the program if it's the case
void run(queue<cmd> &commands, queue<string> &connectors)
{
    if(commands.empty()) return;
    //use the first command "highest priority" 
    string con;
    if(!connectors.empty())con= connectors.front();
    cmd com = commands.front();
    commands.pop();
    
    //execpv and fork process
    cout << "_"<< com.toString() << "_"<< endl;
    if(com.toString() == "exit") exit(0);
    bool ok = exec(com);
    if(ok){ //SUCESS
        if(!connectors.empty() && con == "||" ){
            connectors.pop();
            commands.pop();
        }
        else if(!connectors.empty() && ( con == "&&" || con == ";")){
            connectors.pop();
        }
    } else {//FAIL
        //if command FAILED and is diff form AND then GO ON
        if(!connectors.empty() && con != "&&"){ 
            connectors.pop();
            commands.pop();
        }
        //if command FAILED and is diff form AND then GO ON
        else if(!connectors.empty() && ( con == "||" || con == ";")){ 
            connectors.pop();
        }
    }
    
    run(commands, connectors);
    return ;
}

// returns a string with user and machine name
string getPrompt(){
    char machine[50];
    string user;
    string prompt;
    
    struct passwd *pass = getpwuid(getuid());
    int host = gethostname(machine,50);
    
    if(pass != NULL && host != -1){
        user = pass->pw_name;
        prompt = user + "@" + string(machine)  + "$ ";
    } else{
        prompt = "$ ";
        if(pass == NULL)
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
