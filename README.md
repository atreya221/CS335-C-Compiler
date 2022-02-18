# CS335 Compiler Design Project, IIT Kanpur

This repository contains the code for our course project in Compiler Design (CS335) at IIT Kanpur.

The compiler is being developed in stages. Currently, Stage - 1 of the project has been completed.

## Basic Details

Source Language : C<br>
Implementation Language : C<br>
Target Language : MIPS Assembly<br>

## Building the Project 

To build the project :

```
make
```

To clear the binaries and compiled source : 

```
make clean
```


## Usage Instructions


### Milestone - 2 (Lexer)

In this stage, a standalone scanner is generated in the `bin` directory on building. The scanner, when fed an input program in our source program (C), outputs a table describing the stream of tokens. It lists the line number and the column number where the token starts, and the lexeme associated with the token. To test the scanner,

```
./bin/scanner <path_to_C_file>
```

5 sample test cases have been provided in the tests directory.<br>

NOTE - Preprocessing directives will be ignored by the scanner at this stage of the project.


### Milestone - 3 (Parser)
At this stage, a parser is generated in the `bin` directory on building the project using `make` command.
The parser as of now takes a C file as input and parses it.
If the parse is unsuccessful then it throws error, otherwise it doesn't give any output.

On executing the following command:
```
make plot
```
An `graph.svg` will be generated in the `build` directory. This file contains the LR-Parser Automaton.

### Errors in the grammer  
There is a shift-reduce conflict in the grammer, which is due to the dangling `else` condition.
We were not able to come-up with a fix for this conflict as of now.  
This error emerges when there is an `else` condition which can be linked to multiple `if` conditions, and the compiler is not able to figure that out.

In our compiler, this can be avoided if we use '`{`' '`}`' the body of the `if` and `else` statement inside blocks.  
