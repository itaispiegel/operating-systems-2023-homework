.PHONY: clean

myshell: shell.c myshell.c
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c -o myshell

myshell-debug: shell.c myshell.c
	gcc -g -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 shell.c myshell.c -o myshell-debug

clean:
	rm -f ./myshell ./myshell-debug
