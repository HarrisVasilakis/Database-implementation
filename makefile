prog: main.c HT.c 
	gcc -o prog main.c HT.c BF_64.a -I.
clear:
	rm prog main.o HT.o 
#will work for main.c
