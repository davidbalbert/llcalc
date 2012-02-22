calc:
	gcc calc.c -lm -o calc

calc.c: khash.h

.PHONY: clean
clean:
	rm -f calc
