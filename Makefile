# TOOLS
CC = gcc
CXX = g++
LEX = flex
YACC = bison -y

#This target can keep changing based on final binary required
#TARGET = scanner
# TARGET = parser
TARGET = 3ac

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
CXXFLAGS = -g -Wall -D_CXX
##LFLAGS
YFLAGS = -d
LDFLAGS = -lfl
INCFLAGS = $(addprefix -I, $(INCDIR))

all: $(TARGET)


scanner: grammar patterns
	@mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c $(SRCDIR)/scanner.c -o $(TARGETDIR)/scanner  

parser: grammar patterns
	@mkdir -p $(TARGETDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c $(SRCDIR)/parser.cpp $(SRCDIR)/ast.cpp -o $(TARGETDIR)/parser

symtab: grammar patterns
	@mkdir -p $(TARGETDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c $(SRCDIR)/parser.cpp $(SRCDIR)/ast.cpp  $(SRCDIR)/symtab.cpp $(SRCDIR)/expression.cpp -o $(TARGETDIR)/symtab

3ac: $(BUILDDIR)/symtab.o $(BUILDDIR)/expression.o $(BUILDDIR)/3ac.o
	@mkdir -p $(TARGETDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/symtab.o $(BUILDDIR)/expression.o $(BUILDDIR)/3ac.o -o $(TARGETDIR)/3ac 

$(BUILDDIR)/3ac.o: $(INCDIR)/3ac.h $(INCDIR)/statement.h $(SRCDIR)/3ac.cpp $(SRCDIR)/statement.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS)  $(SRCDIR)/statement.cpp -o $(BUILDDIR)/statement.o
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS)  $(SRCDIR)/3ac.cpp -o $(BUILDDIR)/3ac_.o
	$(LD) -Ur $(BUILDDIR)/statement.o $(BUILDDIR)/3ac_.o -o $(BUILDDIR)/3ac.o

$(BUILDDIR)/expression.o: $(INCDIR)/expression.h $(SRCDIR)/expression.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(SRCDIR)/expression.cpp  -o $(BUILDDIR)/expression.o 

$(BUILDDIR)/symtab.o: patterns grammar $(INCDIR)/ast.h $(INCDIR)/symtab.h $(SRCDIR)/ast.cpp $(SRCDIR)/parser.cpp $(SRCDIR)/symtab.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/y.tab.c -o $(BUILDDIR)/grammar.o 
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c -o $(BUILDDIR)/patterns.o 
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(SRCDIR)/ast.cpp -o $(BUILDDIR)/ast.o 
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(SRCDIR)/parser.cpp -o $(BUILDDIR)/parser.o 
	$(CXX) -c $(CXXFLAGS) $(LDFLAGS) $(INCFLAGS) $(SRCDIR)/symtab.cpp -o $(BUILDDIR)/symtab_.o 
	$(LD) -Ur $(BUILDDIR)/grammar.o $(BUILDDIR)/patterns.o $(BUILDDIR)/ast.o $(BUILDDIR)/parser.o $(BUILDDIR)/symtab_.o -o $(BUILDDIR)/symtab.o


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
