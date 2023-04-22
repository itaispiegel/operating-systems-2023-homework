obj-m := message_slot.o
KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

message_sender: message_sender.c
	gcc -g -O3 -Wall -std=c11 message_sender.c -o message_sender

message_reader: message_reader.c
	gcc -g -O3 -Wall -std=c11 message_reader.c -o message_reader
