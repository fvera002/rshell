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
        void isConnector(char a, char b, queue<string> &connectors)
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
        }
        
        // return whether a or b is a comma
        bool isComma(char a, char b)
        {
            if(a==';' || b==';') return true;
            return false;
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
                for(unsigned i =0; i< input.size()-2; ++i){
                    isConnector(input[i], input[i+1], connectors);
                    if(isComma(input[i], input[i+1]))++i;
                }
            }
            
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep("&;|>");
            tokenizer tokens(input, sep);
            tokenizer::iterator tok_iter;
            
            for (tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter){
                list.push(*tok_iter);
            }
            return list;
        }
};
#endif
