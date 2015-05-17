#Rshell
HW0 CS100 Spring 2015


##Authors and Contributors
######Author

Fernando Donizete Verago Junior

######Contributors

Isadora Maria Mendes de Souza (cp.cpp)

Andrew So (rm.cpp and mv.cpp)

##General Information
####HW0: rshell
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

####HW1: ls
The ls feature was built in order to behave exactly as GNU implementation does, but with some limitations. This behavior can be seen in the [man page](http://unixhelp.ed.ac.uk/CGI/man-cgi?ls) of ls. The main point that this hw1 ls implementation differs from the actual GNU's ls is that it only supports the flags `-a`, `-l` and `-R`. For example: 

`ls -a -l -R`

The flags can be placed in different orders and also together:

`ls -l -a -R`

`ls -lRa`

`ls -al -R`


There is also the case in which you may want to include optional files to your command. This implementation also handles that:

`ls -l file1.txt file2.doc`


Again, the order in which the flags appear in your command line does not matter:

`ls file1.txt file2.doc -l`


####HW2: rshell piping
The first version of rshell was not able to handle piping and I/O reirection. Now that these features are available, commands like `ls -l > files.txt` and `ls -laR | grep Makefile` will work just like an UNIX bash implementation. 

Also, these commands can be mixed up together:

`ls -laR | grep Makefile > ls_make.txt` 

This last command would run `ls -laR | grep Makefile` and redirect its output to the file `ls_make.txt`.


Just like bash, when chaining multiple redirection commands together (e.g. `echo test > f1 > f2 > f3`), even though  rshell will create, open and close all files in the chain, only the last one will contain the output (i.e. only `f3` will contain `test`). 

However, when mixing up input and output redirection (e.g. `./a.out < inputFile > output1 > output2`), rshell has a different aproach. It echos out to the first output file in the chain (i.e. `output1`).


##Installation
####Installation HW0: rshell
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

####Installation HW1: ls 
To be able to install and run the program, it's necessary to follow the same steps as before in hw0 regarding cloning the repository, with some differences upon compiling. The following commands would install correctly the ls feature:
```
$ git clone  https://github.com/fvera002/rshell.git
$ cd rshell
$ git checkout hw1
$ make ls
$ bin/ls
```

####Installation HW2: rshell piping
Since this version just adds new features to the first homework, the installation process is basically the same, except for the version that will to be checked out. Now,  `hw2` needs to be checked out instead of `hw1`. So, the steps will look like this: 

```
$ git clone  https://github.com/fvera002/rshell.git
$ cd rshell
$ git checkout hw2
$ make
$ bin/rshell
```


## Bugs and Limitations
####Bugs HW0: rshell
* This implementation does not support special characters and their features. It only recognizes commands, the given connectors, and the hashtag `#` for comments. Therefore, quotes, parentheses and other special characters would be considered as part of the argument list. It means that the following command would not behave as "usual" (the same way a linux shell behaves):

    `echo "This is a README file" > README.md`

    Instead, it would echo exactly what was written: `"This is a README file" > README.md`

* Running `bin/rshell` by redirecting its input would not work properly. 
    ```
    Do not run:
    bin/rshell < tests.txt
    ```

* It's possible to run a number of commands in a single input; however, there is a limitation of characters accepted depending on the environment. In most of the tests this limit was 4094 characters per entry.

####Bugs HW1: ls
* Output does not line up exactly as it should when running `ls` in a folder with many files.

* The order in which files are displayed is not matching GNU's implementation when files have similar names, such as `ls.cpp` and `ls2.cpp`. 


####Bugs HW2: rshell piping. 
* Chaining multiple output redirections with input redirection does not match behavior that UNIX bash does. Further explanation about this is found above on the overview. 
