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
void ls_a(bool a, char *f)
{
    DIR *dirp;
    if(NULL == (dirp = opendir(f))){
        perror("There was an error with opendir(). ");
        exit(1);
    }
    struct dirent *filespecs;
    errno = 0;
    vector<string> file_list;
    while((filespecs = readdir(dirp)) != NULL){
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

void ls_l(bool a, char *f)
{
    DIR *dirp;
    if(NULL == (dirp = opendir(f))){
        perror("There was an error with opendir(). ");
        exit(1);
    }
    struct dirent *filespecs;
    errno = 0;
    vector<string> file_list;
    while((filespecs = readdir(dirp)) != NULL){
        file_list.push_back(filespecs->d_name);
    }
    sort(file_list.begin(), file_list.end(), compareFileName);
    
    FOR(file_list) { 
        struct stat st;
        if(stat(file_list[i].c_str(),&st) < 0){
            perror("There was an error with stat(). ");
            exit(1);
        } else {
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
            cout << " " << file_list[i] << endl;
        }
        
    }
    
    if(errno != 0){
        perror("There was an error with readdir(). ");
        exit(1);
    }
    if(-1 == closedir(dirp)){
        perror("There was an error with closedir(). ");
        exit(1);
    }
    
    
    
    

}


int main(int argc, char** argv)
{
    if(argc <= 1){
        char dir[] = "./";
        ls_a(false, dir);
    }
    else{
        if(argc == 2 && strcmp(argv[1], "-a")==0){
            char dir[] = "./";
            ls_a(true, dir);
        }
        else if(argc == 2 && strcmp(argv[1], "-l")==0){
            char dir[] = "./";
            ls_l(false, dir);
        }
    }
    
    
    return 0;
}
