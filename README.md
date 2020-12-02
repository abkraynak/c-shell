# c-shell
A simple shell written in C language.
***
## Table of Contents
* [Introduction](#introduction)
* [Usage Examples](#usage-examples)
***
## Introduction
This project is a small, simple shell program for UNIX systems. This program parses a command, checks for the custom commands and operators to handle, and executing them.
The custom functions created are to create aliases and to set variables. 
The program can recognize pipes and input/output redirections. 
***
## Usage and Examples
### Aliases
````
myalias list ls
````
Use aliases to create shortcuts to commands.
### Variables
````
myset x = abc
echo @x
````
Set variables to use in other commands. Use the @ sign to use the variable.
### Redirects
````
ls -al > list.txt
wc < list.txt
````
Use > and < to redirect output and input, respectively. 
### Pipes
````
cat frankenstein.txt | grep beautiful
````
Pipes use the output of the first command to run the second command. Use the | operator between them.
***
## Future Improvements
Some future improvements I have planned and thought-out:
- Use a hashtable to store variables and aliases to more efficiently find and store them
- Make header file to simplify project
- Improve general efficiency of program
