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

#define FOR(x) for (unsigned i =0 ; i < (x).size(); ++i) 
#define PVEC(x) FOR(x) { cout << x[i] << endl ; } 
#define PCMDS(x) FOR(x) { cout << x[i].toString() << endl ; } 

class cmd
{
    private:
        vector<string> argList;
        string comment;
        string input;
        
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
        void extQuotes(string &cmd)
        {
            size_t found;
            found = cmd.find("\"");
            if(found !=string::npos){
                cmd.replace(found, 1, "");
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
        
        // extract the spaces in the beginning and at the end of str
        string trim(string const& str)
        {
            if(str.empty())return str;
        
            size_t first = str.find_first_not_of(' ');
            size_t last  = str.find_last_not_of(' ');
            return str.substr(first, last-first+1);
        }
        
        void tokenize()
        {
            
            char * pch;
            char delim[] = " \n";
            char *aux = new char[input.length()+1];
            strcpy(aux, input.c_str());
            pch = strtok (aux,delim);
            while (pch != NULL){
                argList.push_back(pch);
                pch = strtok (NULL, delim);
            }
            
            if(aux != NULL) delete aux;
        }
        
    public:

        // constructor
        // initialize members
        // extract comments
        // tokenize
        cmd()
        {}
        
        cmd(string command)
        {
            extComment(command);
            extQuotes(command);
            input = trim(command);
            
            tokenize();
            
            //print();
        }
        
        // return the c_string version of the command
        // this function generates memory leak
        // however if called inside child processes it's ok
        char** toArray()
        {
            char ** ret = new char* [argList.size()+1];
            FOR(argList){
                ret[i] = new char[argList.at(i).length()+1];
                strcpy(ret[i], argList.at(i).c_str());
            }
            ret[argList.size()] = NULL;
            
            return ret;
        }
        
        // return the string version of the comman
        string toString()
        {
            return input;
        }
        
        // print commands separeted by _ to check blank spaces
        void print()
        {
            FOR(argList){
                cout << argList.at(i) << "_";
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
                unsigned sz = input.size();
                FOR(input){
                    if(isConnector(input[i]) || isdigit(input[i]) ){
                        string con;
                        if(i+1 < sz && isConnector(input[i+1])){
                            if(i+2 < sz && isConnector(input[i+2])){
                                con = input.substr(i, 3);
                                connectors.push(con);
                                if(isdigit(input[i])) input.erase(input.begin() + i);
                                i += 2;
                            }
                            else {
                                con = input.substr(i, 2);
                                connectors.push(con);
                                if(isdigit(input[i])) input.erase(input.begin() + i);
                                ++i;
                            }                                
                        }
                        else { 
                            if(isdigit(input[i]) == 0){ //not digit
                                con = input.substr(i, 1);
                                connectors.push(con);
                            }
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
