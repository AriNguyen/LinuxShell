all: shell test/test test/out

shell : shell.c 
	gcc -o $@ $^

test/test : test/test.c 
	gcc -o $@ $^

test/out : test/out.c 
	gcc -o $@ $^