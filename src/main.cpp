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
#include <signal.h>

using namespace std;

//to do:
// cat then ^C

void handleIntTerm(int x);
void handleInt(int x);
bool exec(cmd c);
void runPrep(cmd &c);
void run(queue<cmd> &commands, queue<string> &connectors);
vector<cmd> pipesPrep(queue<cmd> &commands, queue<string> &connectors);
bool isRedirect(string con);
bool isOutRed(string con);
int getFlag(string con, int &fd);
bool builtInCD(cmd c);
string replaceHome(string curr);

bool exec2(cmd c)
{
    char **argv = c.toArray();  
    if(-1 == execvp(*argv, argv)){
        cout << c.toString() << ": ";
        perror(string(c.toString() + ": There was an error in execvp()").c_str());
    }
    cout << flush;
    return true;
}

void bkpIO(int &in, int &out)
{
    in = dup(0);
    if (in == -1)
        perror("There was an error in dup");
    out = dup(1);
    if (out == -1)
        perror("There was an error in dup");
}

void restoreIO(int &savedIn, int &savedOut)
{
    if (dup2(savedOut, 1) == -1)
        perror("There was an error in dup2");
    if (dup2(savedIn, 0) == -1)
        perror("There was an error in dup2");
    if (close(savedOut) == -1)
        perror("There was an error in close");
    if (close(savedIn) == -1)
        perror("There was an error in close");
}

bool piping(vector <cmd> &v, const char * ff1, const char * ff2, int flags1, int flags2) {
    
    int savedIn,savedOut;
    bkpIO(savedIn,savedOut);

    int in = 0;
    int output = 1;
    
    if(ff1 != NULL){
        in = open(ff1, flags1, S_IRUSR | S_IWUSR);
        if (in == -1) {
            perror("There was an error with open()");
            //exit(1);
        }
    }
    if(ff2 != NULL ){
        output = open(ff2, flags2, S_IRUSR|S_IWUSR);
        if (output == -1){
            perror("There was an error with open()");
            //exit(1);
        }
    }
    
    int fd[2];
    vector<int> ids;
    //initialize all children expect the last one
    for (unsigned i = 0; i < v.size() - 1; ++i) {
        if (pipe(fd) == -1)
            perror("There was an error in pipe");
            
        int pid = fork();
        if (pid == -1)
            perror("There was an error in fork");
        ids.push_back(pid);
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
    ids.push_back(pid);
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
        FOR(ids) {
            if (waitpid(ids[i], NULL, 0) == -1)
                perror("There was an error in wait");
        }
    }
    restoreIO(savedIn, savedOut);
    cout<< flush;
    return true;
}

int open_f(const char * ff, int flags) 
{
    int fl;
   
    fl = open(ff, flags, S_IRUSR|S_IWUSR);
    if (fl == -1){
        perror("There was an error with open()");
        return -1;
    }
    
    return fl;
}

bool execRedirect(cmd currCmd, int flags, int fd, vector<cmd> &pipes, vector<string> out_list, vector<string> c_list)
{
    // 0 = cin
    // 1 = cout
    // 2 = cerr
    if(fd == 999){
        cmd aux("echo " + out_list[0]);
        vector<cmd> newpipes;
        newpipes.push_back(aux);
        newpipes.push_back(currCmd);
        newpipes.insert( newpipes.end(), pipes.begin(), pipes.end() );
        if(out_list.size() > 1) {
            string f = out_list[out_list.size()-1];
            int fl = getFlag( (c_list[c_list.size()-1]) , fd ); 
            return piping(newpipes, NULL, f.c_str(), 0, fl);
        }
        return piping(newpipes, NULL, NULL, 0, 0);
    }
    else if(pipes.size() > 0){
        vector<cmd> newpipes;
        newpipes.push_back(currCmd);
        newpipes.insert( newpipes.end(), pipes.begin(), pipes.end() );
        string f1= out_list[0];
        if(out_list.size() >1){
            string f2= out_list[out_list.size()-1];
            return piping(newpipes, f1.c_str(), f2.c_str(), getFlag("<", fd), getFlag(c_list.front(),fd));
        }
        return false;
    }
    
    int fd_bkp = fd;
    int savedIn,savedOut;
    bkpIO(savedIn,savedOut); 
    if(close(fd) == -1){
        perror("There was an error with close()");
        restoreIO(savedIn, savedOut);
        return false;
    }
    if(fd == 0 && c_list.size() > 0 && isOutRed(c_list[0])){
        getFlag(c_list[0], fd);
        if(close(fd) == -1){
            perror("There was an error with close()");
            restoreIO(savedIn, savedOut);
            return false;
        }
    }
    
    vector<int> flist;
    int fl;
    bool ret =false;
    
    FOR(out_list){
        if(i==0)fl= open_f(out_list[i].c_str(), flags);
        else fl = open_f(out_list[i].c_str(), getFlag(c_list[i-1], fd));
        if(fl == -1){
            restoreIO(savedIn, savedOut);
            return false;
        } 
        flist.push_back(fl);
    }
    
    
    bool ran =false;
    FOR(flist){
        if(dup2(flist[i], fd_bkp) == -1){
            perror("There was an error with dup2()");
            //exit(1);
        }
        
        if( ((flist.size() == 1) 
            || (fd_bkp == 0 && i==0 && flist.size() > 1) 
            || (fd_bkp == 0 && flist.size() > 1 && i== flist.size() -1)
            || (fd_bkp > 0 && i== flist.size() -1))
            && !ran ){
            ret = exec(currCmd);
            ran = true;
        }
        
        if(fd_bkp == 0 && i != 0 ){
            if( close(flist[i]) == -1){
                perror("There was an error with close(). ");
                //exit(1);
            }
        }
    }
        
    if( fd_bkp == 0 && close(flist[0]) == -1){
        perror("There was an error with close(). ");
        //exit(1);
    }

    restoreIO(savedIn, savedOut);

    return ret;
    
}

bool redirect(queue<cmd> &commands, queue<string> &connectors, int flags, int fd, vector<cmd> &pipes)
{
    //if there's no file passed in, do nothing  
    cmd currCmd;  
    if(pipes.empty()){
        if(commands.size() < 2) return false; 
        currCmd = commands.front();
        commands.pop();
       
    }
    
    string file_name = commands.front().toString();
    if(!commands.empty())commands.pop();
    if(!connectors.empty())connectors.pop();
    //cout << "Printing into: " << file_name << endl;
    bool ret =false;
        
    if(!pipes.empty()) {
        piping(pipes, NULL, file_name.c_str(), 0, flags);
        return true;
    }
    if( (!commands.empty()) && (!connectors.empty()) ){
        string c2 = connectors.front();
        if(c2 == "|"){
            if(!connectors.empty())connectors.pop();
            pipes =  pipesPrep(commands,connectors);
        }
    }
    //cout << "pipes before: " << endl;
    //PCMDS(pipes);
    vector<string> l;
    vector<string> l2;
    l.push_back(file_name);
    while(!connectors.empty() && isOutRed(connectors.front())){
        if(!commands.empty()) l.push_back(commands.front().toString());
        l2.push_back(connectors.front());
        if(!commands.empty())commands.pop();
        if(!connectors.empty())connectors.pop();
    }
    ret = execRedirect(currCmd, flags, fd, pipes, l, l2);
    
    
    
    return ret;
}

int getFlag(string con, int &fd)
{
    const int trunc = O_CREAT|O_WRONLY|O_TRUNC;
    const int append = O_CREAT|O_WRONLY|O_APPEND;
    const int read = O_RDONLY;
    
    if(con == ">" || con == "1>"){
        fd =1;
        return  trunc;
    }
    else if(con == ">>" || con == "1>>"){
        fd = 1;
        return  append;
    }
    else if(con == "2>"){
        fd = 2;
        return  trunc;
    }
    else if(con == "2>>"){
        fd = 2;
        return  append;
    }
    else if(con == "<"){
        fd = 0;
        return  read;
    }
    fd = -1;
    return -1;
}

bool redirectPrep(queue<cmd> &commands, queue<string> &connectors, vector<cmd> pipes)
{
    string con;
    if(!connectors.empty()) con = connectors.front();
    
    
    const int trunc = O_CREAT|O_WRONLY|O_TRUNC;
    const int append = O_CREAT|O_WRONLY|O_APPEND;
    const int read = O_RDONLY;
    
    if(con=="<<<")
        return redirect(commands, connectors, 999, 999, pipes);
    else if(con == ">" || con == "1>")
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
    if(pid == -1){//fork?s return value for an error is -1
        perror("There was an error with fork()");
        exit(1);//there was an error with fork so exit the program and go back and fix it
    }
    else if(pid == 0){//when pid is 0 you are in the child process
        //This is the child process 
        struct sigaction sa;
        sa.sa_handler = handleIntTerm;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; //Restart
        if (sigaction(SIGINT, &sa, NULL) == -1) {
            perror("There was an error in sigaction()");
            exit(1);
        }
        char **argv = c.toArray();  
        if(-1 == execvp(*argv, argv)){
            perror(string(c.toString() + ": There was an error in execvp()").c_str());
        }
        exit(1);
    }
    //if pid is not 0 then we?re in the parent
    //parent process
    else{
        struct sigaction sa;
        sa.sa_handler = handleInt;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; //Restart
        if (sigaction(SIGINT, &sa, NULL) == -1) {
            perror("There was an error in sigaction()");
            exit(1);
        }
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
    cout<<flush;
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

bool isOutRed(string con)
{
    //cout << "--" << con <<endl;
    FOR(con){
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
    if(com.toVector().at(0) == "cd"){
        bool r = builtInCD(com);
        commands.pop();
        run2(commands, connectors, r);
        return;
    }
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

bool builtInCD(cmd c)
{
    vector<string> clist = c.toVector();
    string path;
    if(clist.size() == 1){
        path = getenv("HOME");
    }
    else if(clist.size() == 2){
        if(clist.at(1)== "-")  path = getenv("OLDPWD");
        else path = clist.at(1);
    }
    const char *pwd0 = "PWD";
    char *pwd = getenv(pwd0);
    if(pwd==NULL){
        perror("There was an error in getenv");
        return false;
    }
    
    if(chdir(path.c_str()) == -1){
        perror(string(path +" There was an error in chdir").c_str());
        return false;
    }
    char path2[128];
    char *path3  = getcwd(path2,128);
    if(path3==NULL){
        perror("There was an error in getcwd");
    }
    if(setenv(pwd0, path3, 1) == -1){
        perror("There was an error in setenv");
        return false;
    }
    const char *oldpwd0 = "OLDPWD";
    if(setenv(oldpwd0, pwd, 1) == -1){
        perror("There was an error in setenv");
        return false;
    }
    
    /*
    char *pwd2 = getenv(pwd0);
    char *oldpwd2 = getenv(oldpwd0);
    cout << "curr: " << pwd2 << endl;
    cout << "old: " << oldpwd2 << endl;
    */
    
    char *pwd2 = getenv("PWD");
    if(pwd2==NULL){
        perror("There was an error in getenv");
    } else  cout << pwd2 << endl;
    
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
    if(com.toVector().at(0) == "cd"){
        bool r = builtInCD(com);
        commands.pop();
        run2(commands, connectors, r);
        return;
    }
    
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
    cout<<flush;
    return ;
}

string replaceHome(string curr)
{
    string ret = curr;
    char *hm = getenv("HOME");
    if(hm==NULL){
        perror("There was an error in getenv");
        return ret;
    }
    string home = hm;
    size_t found;
    found = curr.find(home);
    if(found !=string::npos){
        ret.replace(found, home.size()-found, "");
        ret = "~" + ret;
    }
    
    return ret;
}

// returns a string with user and machine name
string getPrompt(){
    char machine[128];
    char dir[128];
    string user;
    string prompt;
    
    struct passwd *pw  = getpwuid(getuid());
    int host = gethostname(machine,128);
    char *pwd = getcwd(dir, 128);
    if(pw != NULL && host != -1 && pwd != NULL){
        user = pw->pw_name;
        prompt = user + "@" + string(machine) + ": " + replaceHome(pwd)  + " $ ";
    } else{
        prompt = "$ ";
        if(pw == NULL)
            perror("There was an error in getpwuid()");
        if(host == -1)
            perror("There was an error in gethostname()");
    }
    return prompt; 

}

void handleInt(int x)
{
    cout<<endl;
    cout<<flush;
}

void handleIntTerm(int x)
{
    cout<<endl<<flush;
    cin.clear();
    if(raise(SIGKILL) != 0){
        perror("There was an error in raise()");
        //exit(1);
    }
}

void handleStop(int x)
{
    if(raise(SIGSTOP) != 0){
        perror("There was an error in raise()");
        //exit(1);
    }
}


// main program
// get input until in a infinite loop
// within functions are responsible for exiting the program
int main() {
    struct sigaction sa;
    sa.sa_handler = handleInt;
    sigemptyset(&sa.sa_mask);
    //sa.sa_flags = SA_RESTART; //Restart
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("There was an error in sigaction()");
        exit(1);
    }
    while(true){
        cin.clear();
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
    //*/
    return 0;
}
