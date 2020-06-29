shell : shell.o tokenizer.o utils.o
	cc shell.o tokenizer.o utils.o -o shell
shell.o : shell.c
	cc shell.c -c -w
tokenizer.o : tokenizer.c tokenizer.h
	cc tokenizer.c -c
utils.o : utils.c utils.h
	cc utils.c -c
clean:
	rm *.o shell -f
