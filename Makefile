calc: calc.c linenoise.h linenoise.c khash.h
	gcc linenoise.c calc.c -lm -o calc

.PHONY: clean
clean:
	rm -f calc
