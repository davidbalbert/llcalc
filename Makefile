CC=gcc

calc: calc.c linenoise.c 
	$(CC) linenoise.c calc.c -lm -o calc

calc.c: khash.h linenoise.h

.PHONY: clean
clean:
	rm -f calc
