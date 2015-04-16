#Rshell
HW0 CS100 Spring 2015

##General Information
This is an implementation of a command shell in which users can input either a simple command or multiple commands at a single entry by using connectors. 

* this is an example of a single command:

    `ls`

* this is an example of mutiple commands separeted by the connector comma:

    `ls ; pwd ; echo test`

There are only 3 conectors supported by rshell:
* `&&`  if a command is followed by `&&`, then the next command is executed only if the first one succeeds
* `||`  if a command is followed by `||`, then the next command is executed only if the first one fails
* `;`   if a command is followed by `;`, then the next command is always executed 

A command can have mutiple arguments, also known as flags. For example:

`ls -a -l -R`

Users can also include comments in a entry by using the character `#`:

`ls -a #this is a comment`

##How to run
To be able to install and run the program, it's necessary to clone the repository, install and run the rshell. The following commands would do the described steps:

`$ git clone  https://github.com/fvera002/rshell.git`

`$ cd rshell`

`$ git checkout hw0`

`$ make`

`$ bin/rshell` 


Upon running, the program will display a prompt waiting for an entry:

`logged_user@machine_name123$`

The same prompt will be displayed every time the program is done executing commands. There is a built in command that can be used to quit the program:

`exit`

It can also be used together with other commands and connectors. For instance:

`ls -l ; pwd; exit`

This last example would run `ls -l`, then `pwd`, so finally it would exit the program. 


## Bugs and Limitations

* This implementation does not support special characters and their features. It only recognizes commands, the given connectors, and the hashtag `#` for comments. Therefore, quotes, parentheses and other special characters would be considered as part of the argument list. It means that the following command would not behave as "usual" (the same way a linux shell behaves):

    `echo "This is README file" > README.md`

    Instead, it would echo exactly what was written: `"This is README file" > README.md`

* Consequently, special signal commonly used in linux are not supported by rshell. Therfore the `^C` signal will not terminate the program; only the `exit` command will.
