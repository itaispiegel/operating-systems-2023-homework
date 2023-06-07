.PHONY: clean

test: test.c queue.c queue.h
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 -pthread test.c -o test -g

clean:
	rm -f test
