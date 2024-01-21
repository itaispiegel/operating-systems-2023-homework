.PHONY: clean
a.out: os.c pt.c
	gcc -O3 -Wall -std=c11 os.c pt.c

os-debug: os.c pt.c
	gcc -Wall -std=c11 -g os.c pt.c -o os-debug

clean:
	rm -f ./a.out ./os-debug
