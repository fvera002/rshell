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

using namespace std;

int main(int argc, char** argv)
{
    char * path;
    if(argc <= 1)
    {
        path = new char[3];
        strcpy(path, "./");        
    }
    else
    {
        path = argv[1];
    }
    DIR *dirp;
    if(NULL == (dirp = opendir(path)))
    {
        perror("There was an error with opendir(). ");
        exit(1);
    }
    struct dirent *filespecs;
    errno = 0;
    vector<string> file_list;
    while(NULL != (filespecs = readdir(dirp)))
    {
        file_list.push_back(filespecs->d_name);
    }
    
    sort(file_list.begin(), file_list.end());
    
    for (unsigned i =0 ; i < file_list.size(); ++i){
        cout << file_list[i] << "  "; 
    }
    if(errno != 0)
    {
        perror("There was an error with readdir(). ");
        exit(1);
    }
    cout << endl;
    if(-1 == closedir(dirp))
    {
        perror("There was an error with closedir(). ");
        exit(1);
    }
    
    
    delete [] path;
    return 0;
}
