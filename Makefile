all: main.o messages.o
	gcc main.o messages.o -o final

main.o: main.c messages.h
	gcc -c main.c

messages.o: messages.c messages.h
	gcc -c messages.c

clean:
	rm -f *.o final