default: chsum

main.o: main.c argtable3.c argtable3.h
	gcc -w -c main.c argtable3.c argtable3.h

chsum: main.o
	gcc -w main.o argtable3.o -o chsum

clean:
	rm -f main.o
	rm -f argtable3.o
	rm -f argtable3.h.gch
	rm -f chsum


