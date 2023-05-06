CC = gcc
CFLAGS = -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11

all: pcc_client pcc_server

pcc_client: pcc_client.c
pcc_server: pcc_server.c

clean:
	rm -f pcc_client pcc_server
