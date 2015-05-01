#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <queue>
#include <string>
#include <algorithm> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <cstring>
#include <iomanip>

using namespace std;

#define FOR(x)  for (unsigned i =0 ; i < (x).size(); ++i) 

// structure is used to print name and and st is used to give info about that specific file
struct stElem{
    struct stat st;
    string nm;
    
    stElem(struct stat st1, string name) : st(st1), nm(name) {}
};

// helper function to sort() to make its ordering as close as possible to GNUs
bool compareFileName(string a, string b) {
    string a1(a);
    string b1(b);
    
    FOR(a) a1[i] = char(tolower(a[i]));
    FOR(b) b1[i] = char(tolower(b[i]));
    
    if(a1[0] == '.') a1 = a1.substr(1);
    if(b1[0] == '.') b1 = b1.substr(1);
        
    return a1 < b1;
}

// print name according to its info in st
string printColor(string &name, struct stat &st){
    string ret ="";
    if ((name[0] == '.') && (S_ISLNK(st.st_mode))) {
        ret += "\033[1;100;36m" + name +"\033[0;00m";
    }
    else if ((name[0] == '.') && S_ISDIR(st.st_mode)) {
        ret += "\033[1;100;34m" + name +"\033[0;00m";
    }
    else if ((name[0] == '.') && (st.st_mode & S_IXUSR)) {
        ret += "\033[1;100;32m" + name +"\033[0;00m";
    }
    else if (name[0] == '.') {
        ret += "\033[1;100;37m" + name +"\033[0;00m";
    }
    else if (S_ISLNK(st.st_mode)) {
        ret += "\033[1;36m" + name +"\033[0;00m";
    }
    else if (S_ISDIR(st.st_mode)) {
        ret += "\033[1;34m" + name +"\033[0;00m";
    }
    else if (st.st_mode & S_IXUSR) {
        ret += "\033[1;32m" + name +"\033[0;00m";
    }
    else {
        ret += name;
    }
    
    return ret;
}

// print ls without any special formatting no flags -l -f
void printLsOnly(vector<string> &file_list)
{
    if(file_list.empty()) return;
    FOR(file_list){
        struct stat st;
        if(lstat(file_list[i].c_str(),&st) < 0){
            cout << __LINE__ << endl;
            perror(string("There was an error with stat(" + file_list[i] + ")").c_str());
            //exit(1);
        } else {
            string name = file_list[i];
            size_t last  = file_list[i].find_last_of('/');
            if( last != string::npos)
                name = file_list[i].substr(last+1);
            if(i == 0){
                 cout << printColor(name, st);
            }
            else {
                cout<< "  " << printColor(name, st);
            }
        }
    }
    cout << endl;
}

// according to flags set names that is going to be run in the current folder
void setNames(vector<string> &file_list, vector<bool> &flags, char *dir)
{
    DIR *dirp;
    if(NULL == (dirp = opendir(dir))){
        perror("There was an error with opendir()");
        exit(1);
    }
    struct dirent *filespecs;
    errno = 0;
    while((filespecs = readdir(dirp)) != NULL){        
        if(flags[0]){
            file_list.push_back(filespecs->d_name);
        } else {
            if(filespecs->d_name[0]!='.')
                file_list.push_back(filespecs->d_name);
        }
    }    
    sort(file_list.begin(), file_list.end(), compareFileName);
    
    if(errno != 0){
        perror("There was an error with readdir()");
        exit(1);
    }
    if(-1 == closedir(dirp)){
        perror("There was an error with closedir()");
        exit(1);
    }
}

//print one line of ls -l
void printLine(struct stat &st, string &name){
    /*
     *file permissions,
     *number of links,
     *owner name,
     *owner group,
     *file size,
     *time of last modification, and
     *file/directory name
    */
    
    cout << ( (S_ISDIR(st.st_mode)) ? "d" : S_ISLNK(st.st_mode)? "l" : "-");
    cout << ( (st.st_mode & S_IRUSR) ? "r" : "-");
    cout << ( (st.st_mode & S_IWUSR) ? "w" : "-");
    cout << ( (st.st_mode & S_IXUSR) ? "x" : "-");
    cout << ( (st.st_mode & S_IRGRP) ? "r" : "-");
    cout << ( (st.st_mode & S_IWGRP) ? "w" : "-");
    cout << ( (st.st_mode & S_IXGRP) ? "x" : "-");
    cout << ( (st.st_mode & S_IROTH) ? "r" : "-");
    cout << ( (st.st_mode & S_IWOTH) ? "w" : "-");
    cout << ( (st.st_mode & S_IXOTH) ? "x" : "-");
    
    cout  << " "<< setw(2) << st.st_nlink;
    
    struct passwd *pw;
    if((pw = getpwuid(st.st_uid))  == NULL){
        perror("There was an error with getpwuid()");
        exit(1);
    }
    cout << " "  << (pw->pw_name); 
    
    struct group *g;
    if( ( g = getgrgid(st.st_gid) ) == NULL ) {
        perror("There was an error with getgrgid()");
        exit(1);
    }
    cout << " " << setw(1) << (g->gr_name); 
    
    cout << " "  << setw(5) << st.st_size;
    
    struct tm * time_st = localtime(&st.st_mtime);     
    if (time_st == NULL){
        perror("There was an error with localtime()");
        exit(1);
    }
    char buf[20];
    if (strftime(buf,20,"%h %d %R",time_st) == 0){
        perror("There was an error with strftime()");
        exit(1);
    }
    cout << " " << setw(12)<< (buf); 
    
    cout << " "<< printColor(name, st) <<endl;
    
}

//print total and all lines of a ls -l command
void printLines(vector<stElem> &lines, long total){
    total = total != 0 ? total / 1024: 0;
    cout << "total " << total <<endl;
    
    FOR(lines){
        printLine(lines[i].st, lines[i].nm);
    }
}

// all logic regarding lsL
void lsL(vector<string> &file_list, char * dir)
{
    vector<stElem> lines; 
    
    long total = 0;
    FOR(file_list) { 
        string sfile = string(dir) + "/" + file_list[i];
        struct stat st;
        if(lstat(sfile.c_str(),&st) < 0){
            perror(string("There was an error with stat(" + sfile + ")").c_str());
            //exit(1);
        } else {
            lines.push_back(stElem(st, file_list[i]));
            total += st.st_blocks*512;
        }
    }
    printLines(lines,total);
}

// all logic regarding ls -R
void lsR(vector<string> &file_list, vector<bool> &flags, string dir)
{
    if(file_list.empty()) return;
    
    vector<string> subs;
    vector<stElem> lines;
    long total = 0;
    
    if(dir == "./") cout << ".:"<< endl;
    else cout << dir << ":"<< endl; 
    
    vector<string> file_list_A;
    
    FOR(file_list) { 
        struct stat st;
        string fl = dir + "/"+ file_list[i];
        
        if(lstat(fl.c_str(),&st) < 0){
            perror(string("There was an error with stat(" + fl + ")").c_str());
            //exit(1);
        } else {
            if(S_ISDIR(st.st_mode)){
                if(file_list[i] != "." && file_list[i] != ".."){
                    subs.push_back(file_list[i]);
                }
            }
            if(flags[1]){
                lines.push_back(stElem(st, file_list[i]));
                total += st.st_blocks*512;
            } else file_list_A.push_back(fl);
        }        
    }
    if( !flags[1] ) printLsOnly(file_list_A);
    else printLines(lines,total);
    
    FOR(subs){
        cout <<endl;
        
        string fl = subs[i];
        if(dir == "./") fl = dir + fl;
        else fl = dir + "/" + fl;
                
        char *dir2 = new char[fl.length()+1];
        strcpy(dir2,fl.c_str());
        
        vector<string> subs2;
        setNames(subs2, flags, dir2);
        lsR(subs2, flags, dir2);
        
        delete [] dir2;
    }

}

// set of flags as true
void allTrue(vector<bool> &flags)
{
    FOR(flags){
        flags[i] = true;
    }
}

// set each flag according to the args. 
// Take care of all possible choices
void setFlags(int argc, char** argv, vector<bool> &f)
{
    // [0] = -a
    // [1] = -l
    // [2] = -R
    f[0] = false;
    f[1] = false;
    f[2] = false;
    for(int i = 1; i< argc; ++i){
        if(strcmp(argv[i], "-a")==0) f[0] = true;
        else if(strcmp(argv[i], "-l")==0) f[1] = true;
        else if(strcmp(argv[i], "-R")==0) f[2] = true;
        else if(strcmp(argv[i], "-la")==0) f[1] = true, f[0] = true; 
        else if(strcmp(argv[i], "-al")==0) f[0] = true, f[1] = true;
        else if(strcmp(argv[i], "-Rl")==0) f[2] = true, f[1] = true;
        else if(strcmp(argv[i], "-lR")==0) f[1] = true, f[2] = true;
        else if(strcmp(argv[i], "-aR")==0) f[0] = true, f[2] = true;
        else if(strcmp(argv[i], "-Ra")==0) f[2] = true, f[0] = true;
        else if(strcmp(argv[i], "-lRa")==0) allTrue(f);
        else if(strcmp(argv[i], "-laR")==0) allTrue(f);
        else if(strcmp(argv[i], "-Rla")==0) allTrue(f);
        else if(strcmp(argv[i], "-Ral")==0) allTrue(f);
        else if(strcmp(argv[i], "-aRl")==0) allTrue(f);
        else if(strcmp(argv[i], "-alR")==0) allTrue(f);
    }
    
}

// check whether an arg is a flag
bool isFlag(char * arg)
{
    return  (strcmp(arg, "-a")==0)
         || (strcmp(arg, "-l")==0)
         || (strcmp(arg, "-R")==0)
         || (strcmp(arg, "-la")==0) 
         || (strcmp(arg, "-al")==0)
         || (strcmp(arg, "-Rl")==0)
         || (strcmp(arg, "-lR")==0)
         || (strcmp(arg, "-aR")==0)
         || (strcmp(arg, "-Ra")==0)
         || (strcmp(arg, "-lRa")==0)
         || (strcmp(arg, "-laR")==0)
         || (strcmp(arg, "-Rla")==0)
         || (strcmp(arg, "-Ral")==0)
         || (strcmp(arg, "-aRl")==0)
         || (strcmp(arg, "-alR")==0);
}

// check all args if it's ar optional file put it onto the list
void setOpFiles(int argc, char** argv, vector<string> &files)
{
    for(int i = 1; i< argc; ++i){
        if( ! isFlag(argv[i])) files.push_back(argv[i]);
    }
    sort(files.begin(), files.end(), compareFileName);
}

// No optional File was passed, do the logic in the current fodler
void noOpFiles(vector<bool> &flags)
{
    vector<string> file_names;
    char dir[] ="./";
    setNames(file_names, flags, dir);
    
    // handles:
    // ls
    // ls -a
    if(!flags[1] && !flags[2]){ 
        printLsOnly(file_names);
    }
    
    // handles:
    // ls -l
    // ls -l -a
    else if(flags[1] && !flags[2]){ // if [ls -l] or [ls -l -a] was passed in 
        // flag -a does not matter since setNames() has already taken care of it
        lsL(file_names, dir);
    }
    
    // handles:
    // ls -R
    // ls -R -a
    // ls -R -a -l
    else if(flags[2]){ 
        // flag -a does not matter since setNames() is taking care of it
        lsR(file_names, flags, "./");
    }    
}

// takes care of optional files that are directories
void opFilesDir(vector<bool> &flags, vector<string> &op_files)
{
    //vector<string> dirs;
    
    FOR(op_files){
        if (!flags[2])cout << endl << op_files[i] << ":"<< endl;
        if (flags[2])cout<< endl;
       
        vector<string> file_names;
        string sdir = "./" + op_files[i];
        
        char *dir = new char[sdir.length()+1];
        strcpy(dir, sdir.c_str());
        
        setNames(file_names, flags, dir);
        
        if (flags[1] && !flags[2]) lsL(file_names, dir);
        else if (!flags[1] && !flags[2]){
            FOR(file_names){
                file_names[i] = sdir + "/" + file_names[i];
            }
            
            printLsOnly(file_names);
        }
        else if (flags[2]) lsR(file_names, flags, dir);
        
        delete [] dir;
    }
    
    //if(flags[2]) opFilesDir(flags, dirs);
}

// takes care of optional files that are directories
void ls_opFilesNotDir(vector<bool> &flags, vector<string> &op_files)
{
    if(op_files.empty())return;
    if (!flags[1]){
        printLsOnly(op_files);
        return;
    } 
    
    FOR(op_files){
        if (flags[1]){
            struct stat st;
            if(lstat(op_files[i].c_str(),&st) < 0){
                cout << __LINE__ << endl;
                perror(string("There was an error with stat(" + op_files[i] + ")").c_str());
                //exit(1);
            } else {
                printLine(st, op_files[i]);
            } 
        }        
    }
}

// do logic reagarding optional files
void opFiles(vector<bool> &flags, vector<string> &op_files)
{
    if(op_files.empty())return;
    vector<string> is_dir;
    vector<string> not_dir;
    
    // handles everything, except for -R:
    // ls [op1..files]
    // ls -a  [op1..files]
    // ls -l [op1..files]
    // ls -l -a [op1..files]
    FOR(op_files) { 
        struct stat st;
        if(lstat(op_files[i].c_str(),&st) < 0){
            perror(string("There was an error with stat(" + op_files[i] + ")").c_str());
    
            //exit(1);
        } else {
            if(S_ISDIR(st.st_mode)){
                is_dir.push_back(op_files[i]);
            }
            else {
                not_dir.push_back(op_files[i]);
            }
        } 
    }
    
    ls_opFilesNotDir(flags, not_dir);
    
    // ls -l -a -R [op1..files]
    opFilesDir(flags, is_dir);
}

int main(int argc, char** argv)
{
    
    vector<bool> flags(3);
    setFlags(argc, argv, flags);
    
    vector<string> op_files;
    setOpFiles(argc, argv, op_files);
    
    if(op_files.empty()){
        noOpFiles(flags);
    } else {
        opFiles(flags, op_files);
    }
    
    return 0;
    
}
