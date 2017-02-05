CC=gcc
CFLAGS = -g 
# uncomment this for SunOS
# LIBS =  -lsocket -lnsl

all: udp-send udp-recv

udp-send: udp-send.c port.h 
	$(CC) -g -o udp-send -D_GNU_SOURCE udp-send.c $(LIBS) 

udp-recv: udp-recv.c port.h
	$(CC) -g -o udp-recv -D_GNU_SOURCE udp-recv.c $(LIBS) 

clean:
	rm -f udp-send udp-recv
