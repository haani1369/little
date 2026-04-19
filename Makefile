.PHONY: little
little:
	gcc little.c -o little -lncurses
	./little

.PHONY: clean
clean:
	rm -f little

