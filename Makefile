all: little seahorse

.PHONY: little
little:
	gcc -pedantic little.c -o little -lncurses

.PHONY: seahorse
seahorse:
	gcc -pedantic seahorse.c -o seahorse -lncurses

.PHONY: clean
clean:
	rm -f little seahorse

