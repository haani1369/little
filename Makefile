make run: little
	./little

.PHONY: little
little:
	gcc little.c -o little -lncurses

.PHONY: clean
clean:
	rm -f little obfuscated_little.c

