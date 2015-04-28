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

// TODO:
// total 23 when run ls -l
// op files
// formatting ls -a long lists
// color

using namespace std;

#define FOR(x)  for (unsigned i =0 ; i < (x).size(); ++i) 

bool compareFileName(string a, string b) {
    string a1(a);
    string b1(b);
    a1[0] = char(tolower(a[0]));
    b1[0] = char(tolower(b[0]));
        
    return a1 < b1;
}

void print_ls_a(vector<string> &file_list)
{
    if(file_list.empty()) return;
    FOR(file_list){
        if(i == 0) cout<< file_list[i] ;
        else cout<< "  " << file_list[i] ; 
    }
    cout << endl;
}

void set_file_names(vector<string> &file_list, vector<bool> &flags, char *dir)
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

void print_line(struct stat &st, string &name){
    /*
     *file permissions,
     *number of links,
     *owner name,
     *owner group,
     *file size,
     *time of last modification, and
     *file/directory name
    */


    cout << ( (S_ISDIR(st.st_mode)) ? "d" : "-") ;
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
    cout << " "  << (g->gr_name); 
    
    cout << " "  << setw(5) << st.st_size;
    
    struct tm * time_st = localtime(&st.st_mtime);     
    if (time == NULL){
        perror("There was an error with localtime()");
        exit(1);
    }
    char buf[20];
    if (strftime (buf,20,"%h %d %R",time_st) == 0){
        perror("There was an error with strftime()");
        exit(1);
    }
    cout << " " << setw(12)<< (buf); 
    
    cout << " "  << name << endl;
    
    
}

void ls_l(vector<string> &file_list, char * dir)
{
    
    FOR(file_list) { 
        string sfile = string(dir) + "/" + file_list[i];
        struct stat st;
        if(stat(sfile.c_str(),&st) < 0){
            perror("There was an error with stat()");
            exit(1);
        } else {
            print_line(st, file_list[i]);
        }
    }
}

void ls_R(vector<string> &file_list, vector<bool> &flags, string dir)
{
    if(file_list.empty()) return;
    vector<string> subs;
    if(dir == "./") cout << ".:"<< endl;
    else cout << dir << ":"<< endl; 
    if( !flags[1] ) print_ls_a(file_list);
    FOR(file_list) { 
        struct stat st;
        string fl = dir + "/"+ file_list[i];
        
        if(stat(fl.c_str(),&st) < 0){
            perror("There was an error with stat()");
            exit(1);
        } else {
            if(S_ISDIR(st.st_mode)){
                if(!flags[0] || file_list[i].at(0) != '.'){
                    subs.push_back(file_list[i]);
                }
            }
            if(flags[1]) print_line(st, file_list[i]);
        }        
    }
    
    FOR(subs){
        cout <<endl;
        
        string fl = subs[i];
        if(dir == "./") fl = dir + fl;
        else fl = dir + "/" + fl;
        
        char dir2[fl.size()+1];
        
        strcpy(dir2,fl.c_str());
        
        vector<string> subs2;
        set_file_names(subs2, flags, dir2);
        ls_R(subs2, flags, dir2);
    }

}


void alltrue(vector<bool> &flags)
{
    FOR(flags){
        flags[i] = true;
    }
}

void set_flags(int argc, char** argv, vector<bool> &f)
{
    // [0] = -a
    // [1] = -l
    // [2] = -R
    f[0] = false;
    f[1] = false;
    f[2] = false;
    for(unsigned i = 1; i< argc; ++i){
        if(strcmp(argv[i], "-a")==0) f[0] = true;
        else if(strcmp(argv[i], "-l")==0) f[1] = true;
        else if(strcmp(argv[i], "-R")==0) f[2] = true;
        else if(strcmp(argv[i], "-la")==0) f[1] = true, f[0] = true; 
        else if(strcmp(argv[i], "-al")==0) f[0] = true, f[1] = true;
        else if(strcmp(argv[i], "-Rl")==0) f[2] = true, f[1] = true;
        else if(strcmp(argv[i], "-lR")==0) f[1] = true, f[2] = true;
        else if(strcmp(argv[i], "-aR")==0) f[0] = true, f[2] = true;
        else if(strcmp(argv[i], "-Ra")==0) f[2] = true, f[0] = true;
        else if(strcmp(argv[i], "-lRa")==0) alltrue(f);
        else if(strcmp(argv[i], "-laR")==0) alltrue(f);
        else if(strcmp(argv[i], "-Rla")==0) alltrue(f);
        else if(strcmp(argv[i], "-Ral")==0) alltrue(f);
        else if(strcmp(argv[i], "-aRl")==0) alltrue(f);
        else if(strcmp(argv[i], "-alR")==0) alltrue(f);
    }
    
}

bool is_flag(char * arg)
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

void set_files(int argc, char** argv, vector<string> &files)
{
    for(unsigned i = 1; i< argc; ++i){
        if( ! is_flag(argv[i])) files.push_back(argv[i]);
    }
    sort(files.begin(), files.end(), compareFileName);
}

void no_op_files(vector<bool> &flags)
{
    vector<string> file_names;
    char dir[] ="./";
    set_file_names(file_names, flags, dir);
    
    // handles:
    // ls
    // ls -a
    if(!flags[1] && !flags[2]){ 
        print_ls_a(file_names);
    }
    
    // handles:
    // ls -l
    // ls -l -a
    else if(flags[1] && !flags[2]){ // if [ls -l] or [ls -l -a] was passed in 
        // flag -a does not matter since set_file_names() has already taken care of it
        ls_l(file_names, dir);
    }
    
    // handles:
    // ls -R
    // ls -R -a
    // ls -R -a -l
    else if(flags[2]){ 
        // flag -a does not matter since set_file_names() is taking care of it
        ls_R(file_names, flags, "./");
    }    
}

void ls_l_op(vector<bool> &flags, vector<string> &op_files)
{

    FOR(op_files) { 
        struct stat st;
        if(stat(op_files[i].c_str(),&st) < 0){
            perror("There was an error with stat()");
            exit(1);
        } else {
            if(S_ISDIR(st.st_mode)){
                vector<string> file_names;
                cout << op_files[i] << ":"<< endl;
                string sdir = "./" + op_files[i];
                const int sz= sdir.size()+1;
                char dir[sz];
                strcpy(dir, op_files[i].c_str());
                set_file_names(file_names, flags, dir);
                ls_l(file_names, dir);
            }
            print_line(st, op_files[i]);
        } 
    }
}

void run_op_files(vector<bool> &flags, vector<string> &op_files)
{
    /*cout << "op files: " << endl;
    FOR(op_files){
        cout << op_files[i] << endl;
        
    }*/
    
    // handles:
    // ls -l
    // ls -l -a
    if(flags[1] && !flags[2]){
        ls_l_op(flags, op_files);
    }
}

int main(int argc, char** argv)
{
    
    vector<bool> flags(3);
    set_flags(argc, argv, flags);
    
    vector<string> op_files;
    set_files(argc, argv, op_files);
    
    if(op_files.empty()){
        no_op_files(flags);
    } else {
        run_op_files(flags, op_files);
    }
    
    
    return 0;
    
}
