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
#include <string>
#include <algorithm> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
        cout << file_list[i] << "  "; 
    }
    cout << endl;
}

void print_ls(vector<string> &file_list)
{
    if(file_list.empty()) return;
    FOR(file_list){
        if(file_list[i].at(0)!='.')
            cout << file_list[i] << "  "; 
    }
    cout << endl;
}

//runs ls or ls -a in the file/directory f
void ls_path(bool a, char *f)
{
    DIR *dirp;
    if(NULL == (dirp = opendir(f))){
        perror("There was an error with opendir(). ");
        exit(1);
    }
    struct dirent *filespecs;
    errno = 0;
    vector<string> file_list;
    while(NULL != (filespecs = readdir(dirp))){
        file_list.push_back(filespecs->d_name);
    }
    
    sort(file_list.begin(), file_list.end(), compareFileName);
    
    a? print_ls_a(file_list) : print_ls(file_list);
    
    if(errno != 0){
        perror("There was an error with readdir(). ");
        exit(1);
    }
    if(-1 == closedir(dirp)){
        perror("There was an error with closedir(). ");
        exit(1);
    }
}

void run_curr(char *p)
{
    p = new char[3];
    strcpy(p, "./");
}

int main(int argc, char** argv)
{
    char * path;
    if(argc <= 1){
        path = new char[3];
        strcpy(path, "./");
        ls_path(false, path);
    }
    else
    {
        if(argc == 2 && strcmp(argv[1], "-a")==0){
            run_curr(path);
            path = new char[3];
            strcpy(path, "./");
            ls_path(true, path);
        }
    }
    
    delete [] path;
    return 0;
}
