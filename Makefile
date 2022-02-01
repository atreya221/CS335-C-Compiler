# TOOLS
CC = gcc
LEX = flex

#This target can keep changing based on final binary required
TARGET = scanner

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

# FLAGS
CFLAGS = -g -Wall
##LFLAGS
YFLAGS = -d
LDFLAGS = -lfl
INCFLAGS = $(addprefix -I, $(INCDIR))

all: $(TARGET)

scanner: patterns
	@mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCFLAGS) $(BUILDDIR)/lex.yy.c $(SRCDIR)/scanner.c -o $(TARGETDIR)/scanner  

patterns:
	$(LEX) $(PATTERNS)
	@mkdir -p $(BUILDDIR)
	@mv lex.yy.c $(BUILDDIR)/.

.PHONY: clean

clean:
	rm -rf $(BUILDDIR) $(TARGETDIR)
