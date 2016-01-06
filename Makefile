# :tabSize=3:indentSize=3:folding=indent:
# $Id$
EXE = clac.exe
SRC = clac.c
OBJ := $(patsubst %.c,%.o,$(SRC))
LIB = -lkernel32 -luser32
OPT = -Wall -O2

.PHONY: all
all: $(EXE)

$(EXE): $(OBJ)
	gcc $(OPT) -o $@ $(OBJ) $(LIB)

%.o: %.c
	gcc $(OPT) -c $< -o $@

.PHONY: clean
clean:
	CMD /C DEL $(EXE)
	CMD /C DEL *.o
