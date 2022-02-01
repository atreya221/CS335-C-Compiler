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