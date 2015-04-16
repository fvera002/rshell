#ifndef __CMD_H__
#define __CMD_H__
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <queue>
using namespace std;

class cmd
{
    private:
        char *buffer;
        vector<string> v_argList;
        char **c_argList;
        string comment;
        string input;
        
        void print(int argc)
        {
            int i = 0;int j = 0;
            while(i < argc-1)
            {
                j=0;
                while(*(c_argList[i]+j) != '\0')
                {
                    printf("arg[%d][%d] : %c\n", i,j,*(c_argList[i]+(j)));
                    j++;
                } 
                i++;
            }
        }
        
        void tokenize()
        {
            int argc;
            char delim[] = " \n";
            c_argList[0] = strtok(buffer, delim);
            //v_argList.push(c_argList[0]);
        
            for(argc=1; c_argList[argc-1]; argc++){
                c_argList[argc] = strtok(NULL, delim);
                v_argList.push_back(c_argList[argc-1]);
            }
        }
        
        void extComment(string &cmd)
        {
            size_t found;
            found = cmd.find("#");
            if(found !=string::npos){
                comment = cmd;
                cmd.replace(found, cmd.size()-found, "");
                comment.replace(0, found+1, "");
            }
        }
        
        bool isConnector(char a, char b)
        {
            if(a==';') return true;
            if(a=='|' && b=='|') return true;
            if(a=='&' && b=='&') return true;
            return false;
        }
        
        string trim(string const& str)
        {
            if(str.empty())return str;
        
            size_t first = str.find_first_not_of(' ');
            size_t last  = str.find_last_not_of(' ');
            return str.substr(first, last-first+1);
        }
        
    public:
        cmd(string command)
        {
            extComment(command);
            input = trim(command);
            buffer = new char[command.size()+1];
            c_argList = new char* [command.size()+1];
            strcpy(buffer, command.c_str());
            
            tokenize();
            //print();
        }
        
        char** toArray()
        {
            return c_argList;
        }
        
        string toString()
        {
            return input;
        }
        
        void print()
        {
            for(unsigned i=0; i< v_argList.size(); ++i){
                cout << v_argList.at(i) << "_";
            }
            cout <<endl;
        }
        
        queue<cmd> split(queue<string> &connectors)
        {
            queue<cmd> list;
            int i =0;
            string newCmd;
            string con;
            for(unsigned j=0; j< input.size()-1; ++j){
                if(isConnector(input[j], input[j+1])){
                    newCmd = input.substr(i,j-i); 
                    list.push(cmd(newCmd));
                    int nx = j+1;
                    if(input.at(nx)== '|' || input.at(nx)== '&' )++nx;
                    con = input.substr(j,nx-j);
                    connectors.push(con);
                    i= nx;
                }
                
            }
            newCmd = input.substr(i); 
            list.push(cmd(newCmd));
            return list;
        }
        
#endif
