CFLAGS = -std=c17 -std=gnu17 -Wall -Wextra

all:
	gcc -c $(CFLAGS) traceroute.c -o traceroute.o
	gcc -c $(CFLAGS) recieve.c -o recieve.o
	gcc -c $(CFLAGS) send.c -o send.o
	gcc $(CFLAGS) traceroute.o recieve.o send.o -o traceroute
clean:
	rm -f *.o
distclean: clean
	rm -f traceroute