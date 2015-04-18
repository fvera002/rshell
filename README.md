#Rshell
HW0 CS100 Spring 2015


##Authors and Contributors
Fernando Donizete Verago Junior


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

When mixing up connectors in a single entry, rshell group the commands from left to right. For example:

`echo a || echo b && echo c && echo d`

This command would return: 
```
a
c
d
```
Since `echo a` succeed, `echo b` is not executed. Then `echo c` is run because the previous command failed. Finally, `echo d` is executed also because `echo c` succeed. 


##Installation
To be able to install and run the program, it's necessary to clone the repository, run the make command, then finally run rshell that is going to be located in the bin folder. The following commands would do the described steps:
```
$ git clone  https://github.com/fvera002/rshell.git
$ cd rshell
$ git checkout hw0
$ make
$ bin/rshell
```


Upon running, the program will display a prompt waiting for an entry:

`logged_user@machine_name123$`

The same prompt will be displayed every time the program is done executing commands. There is a built in command that can be used to quit the program:

`exit`

It can also be used together with other commands and connectors. For instance:

`ls -l ; pwd; exit`

This last example would run `ls -l`, then `pwd`, so finally it would exit the program. 


## Bugs and Limitations

* This implementation does not support special characters and their features. It only recognizes commands, the given connectors, and the hashtag `#` for comments. Therefore, quotes, parentheses and other special characters would be considered as part of the argument list. It means that the following command would not behave as "usual" (the same way a linux shell behaves):

    `echo "This is a README file" > README.md`

    Instead, it would echo exactly what was written: `"This is a README file" > README.md`

* Running `bin/rshell` by redirecting its input would not work properly. 
    ```
    Do not run:
    bin/rshell < tests.txt
    ```

* It's possible to run a number of commands in a single input; however, there is a limitation of characters accepted depending on the environment. In most of the tests this limit was 4094 characters per entry.

* The use of `\` may cause undefined behavior. 
