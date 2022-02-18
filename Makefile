# TOOLS
CC = gcc
LEX = flex
YACC = bison -y

#This target can keep changing based on final binary required
#TARGET = scanner
TARGET = parser

#DIRECTORIES

##Source code
SRCDIR = src

##Compiled source files
BUILDDIR = build
INCDIR = inc

##Contain final executable
TARGETDIR = bin

#Define some standard files that we use
PATTERNS=$(SRCDIR)/pattern.l
GRAMMAR=$(SRCDIR)/grammar.y

# FLAGS
CFLAGS = -g -Wall -D_CC

##LFLAGS
YFLAGS = -d
LDFLAGS = -lfl
INCFLAGS = $(addprefix -I, $(INCDIR))

all: $(TARGET)


#scanner: grammar patterns
#	@mkdir -p $(TARGETDIR)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c $(SRCDIR)/scanner.c -o $(TARGETDIR)/scanner  

parser: grammar patterns
	@mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c $(SRCDIR)/parser.c -o $(TARGETDIR)/parser 


grammar:
	$(YACC) $(YFLAGS) $(GRAMMAR)
	@mkdir -p $(BUILDDIR)
	@mv y.tab.c $(BUILDDIR)/.
	@mv y.tab.h $(INCDIR)/.

patterns:
	$(LEX) $(PATTERNS)
	@mkdir -p $(BUILDDIR)
	@mv lex.yy.c $(BUILDDIR)/.

plot:
	$(YACC) -v $(GRAMMAR)
	python3 src/graph_generator.py
	sfdp -v -Goverlap=scale -Tsvg graph.dot -o graph.svg
	@rm graph.dot y.tab.c y.output

clean:
	rm -rf $(BUILDDIR) $(TARGETDIR) *.dot *.output y.tab.c
