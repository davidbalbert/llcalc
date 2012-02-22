LDFLAGS=-lm

calc: calc.c linenoise.c 

calc.c: khash.h linenoise.h

.PHONY: clean
clean:
	rm -f calc
