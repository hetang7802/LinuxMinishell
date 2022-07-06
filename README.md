# LinuxMinishell
A simple replication of linux minishell using c++. 


programming language used: c++
operating system: linux/ubuntu

some common commands like man, rm, cal, clear ,ls ,cat, less, more, head, chmod, grep, touch, rmdir, 
wc, pwd, alias, date,  etc. are included and can be used as in the actual shell
					
---------------------

Commands to compile and run: 
	i) chmod +x 200101076.cpp
	ii)g++ 200101076.cpp -o exe
	iii)./exe

![image](https://user-images.githubusercontent.com/83627593/177609169-507af3eb-c8e6-4e75-af05-2f7c06c56c0f.png)

![image](https://user-images.githubusercontent.com/83627593/177608731-0b6295c8-999c-475d-b677-3e88e63274ef.png)

---------------------

Specifications:


For env variable: 

To set an env variable use setenv name=value
	example: setenv USER=STUDENT
	
$(envvariable) works only in echo $(envvariable)
	example: echo $HOME    or    echo $PATH
	
Cannot use something like echo $HOME >outputfile.txt 
But printenv HOME >outputfile.txt works fine and also echo $HOME works fine

---------------------

For redirection:

If single redirection is there, the filename should be the last command
	example: cat <input.txt 	or 		echo hello >output.txt or echo hello >>output.txt

In case of mulitple redirection the input file should be second last while the output/append file should be last command
	example: cat <input.txt >output.txt		 or		 cat <input.txt >>output.txt

--------------------

Internal and external commands as well as piping work fine as per the standard shell syntax

For cd: 
regular syntax as linux shell

For history:
history can be viewed by the command "history"(without quotes)
history is stored in vector and is limited to the runtime of shell
cannot use history with pipe/redirection

For termination:
both exit and quit can be used as commands to come out of the shell
