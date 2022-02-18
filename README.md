# CS335 Compiler Design Project, IIT Kanpur

This repository contains the code for our course project in Compiler Design (CS335) at IIT Kanpur.

The compiler is being developed in stages. Currently, Stage - 1 of the project has been completed.

# Basic Details

Source Language : C<br>
Implementation Language : C<br>
Target Language : MIPS Assembly<br>

# Building the Project 

To build the project :

```
make
```

To clear the binaries and compiled source : 

```
make clean
```


# Usage Instructions


## Milestone - 2 (Lexer)

In this stage, a standalone scanner is generated in the `bin` directory on building. The scanner, when fed an input program in our source program (C), outputs a table describing the stream of tokens. It lists the line number and the column number where the token starts, and the lexeme associated with the token. To test the scanner,

```
./bin/scanner <path_to_C_file>
```

5 sample test cases have been provided in the tests directory.<br>

NOTE - Preprocessing directives will be ignored by the scanner at this stage of the project.


## Milestone - 3 (Parser)
At this stage, a `parser` is generated in the `bin` directory on building the project using `make` command.  

Execute the parser using the following command
```
$./bin/parser ./tests/<test-name>.c
```
The parser as of now takes a C file as input and parses it.
If the parse is unsuccessful then it throws error, otherwise it doesn't give any output.

### `graph.svg` file
This file contains the states and the edges in form of a graph.
Currently edge labels are not visible. But after zooming in and hovering the mouse over the edge, the `edge label` along with the start and end state of the edge will be visible.

You can also generate graph with edge labels visible, folow the instructions below.
### For generating the LR Parser Automata:

##### 1) Execute the following command
```
$make plot
```
##### 2) Prompt will appear like below. If you want curved lines then enter `y` else enter `n`.
```
Do you want splines in your graph? (y/n)
```
If you enter `y`, then the graph generation will take around 2-3 minutes, so wait till the graph is generated.  
Otherwise it will be generated instantly.
##### 3) Another prompt will appear. If you want the edge labels to be visible on the graph then enter `y` else enter `n`.
```
Do you want edge labels to be visible? (y/n)
```
If you enter `n`, then a graph with no visible edge labels will be generated in root folder.   
### On hovering over the edge, you will see the edge label.

A `graph.svg` file will be generated in the root folder.


## Test Files:
The grammar has been designed such that the C program must contain all the declarations first and then the statements.  
If you does not do so, then the parser will throw error.
## Errors in the grammar  
There is a shift-reduce conflict in the grammar, which is due to the dangling `else` condition.
We were not able to come-up with a fix for this conflict as of now.  
This error emerges when there is an `else` condition which can be linked to multiple `if` conditions, and the compiler is not able to figure that out.

In our compiler, this can be avoided if we use '`{`' '`}`'. We need to write the body of the `if` and `else` statement inside blocks while writing C program. 
## References
For Grammar: https://www.lysator.liu.se/c/ANSI-C-grammar-y.html  
For Lexer: https://www.lysator.liu.se/c/ANSI-C-grammar-l.html