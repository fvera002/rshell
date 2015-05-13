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
#include <boost/tokenizer.hpp>
using namespace std;

class cmd
{
    private:
        char *buffer;
        vector<string> v_argList;
        char **c_argList;
        string comment;
        string input;
        
        // prints c_argList showing each char
        void print(int argc)
        {
            int i = 0;int j = 0;
            while(i < argc-1){
                j=0;
                while(*(c_argList[i]+j) != '\0'){
                    printf("arg[%d][%d] : %c\n", i,j,*(c_argList[i]+(j)));
                    j++;
                } 
                i++;
            }
        }
        
        // split the buffer using space as reference
        // store each piece in c_argList and v_argList
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
        
        // extract comment from cmd and save it in comment
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
        
        // return whether a and b together make a connector
        
        bool isConnector(char a)
        {
            return a == ';' ||
                   a == '|' || 
                   a == '&' || 
                   a == '<' || 
                   a == '>' ;
        }
        
        int isConnector(char a, char b, queue<string> &connectors)
        {
            string con;
            if(a==';' || b==';') con = ";";
            else if(a=='|' && b=='|') con = "||";
            else if(a=='|' && b!='|') con = "|";
            else if(a!='|' && b=='|') con = "|";
            else if(a=='&' && b=='&') con = "&&";
            else if(a=='>' && b !='>') con = ">";
            else if(a!='>' && b =='>') con = ">";
            else if(a=='>' && b =='>') con = ">>";
            
            if(!con.empty()){
                connectors.push(con);
            }
            
            return con.size();
        }
        
        // extract the spaces in the beginning and at the end of str
        string trim(string const& str)
        {
            if(str.empty())return str;
        
            size_t first = str.find_first_not_of(' ');
            size_t last  = str.find_last_not_of(' ');
            return str.substr(first, last-first+1);
        }
        
    public:

        // constructor
        // initialize members
        // extract comments
        // split/tokenize
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
        
        // return the c_string version of the command
        char** toArray()
        {
            return c_argList;
        }
        
        // return the string version of the command
        string toString()
        {
            return input;
        }
        
        // print commands separeted by _ to check blank spaces
        void print()
        {
            for(unsigned i=0; i< v_argList.size(); ++i){
                cout << v_argList.at(i) << "_";
            }
            cout <<endl;
        }
       
        // split commands according to the connectors
        // store the new pieces into a queue
        // store the connectors inorder in another queue of connectors
        queue<cmd> split(queue<string> &connectors)
        {
            queue<cmd> list;
            if(input.empty()) return list;
            if(input.size()>2){
                int sz = input.size()-2;
                for(int i =0; i< sz; ++i){
                    if(isConnector(input[i])){
                        string con;
                        if(i+1 < sz && isConnector(input[i+1])){
                            if(i+2 < sz && isConnector(input[i+2])){
                                con = input.substr(i, 3);
                                connectors.push(con);
                                i += 2;
                            }
                            else {
                                con = input.substr(i, 2);
                                connectors.push(con);
                                ++i;
                            }                                
                        }
                        else { 
                            con = input.substr(i, 1);
                            if(!trim(con).empty())
                                connectors.push(con);
                        }
                    }
                    
                }
            }
            
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep("&;|><");
            tokenizer tokens(input, sep);
            tokenizer::iterator tok_iter;
            
            for (tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter){
                list.push(*tok_iter);
            }
            return list;
        }
};
#endif
