.PHONY: clean
pt:
	gcc -O3 -Wall -std=c11 os.c pt.c

clean:
	rm -f ./a.out