CC=gcc
CFLAGS=-g -pedantic -Wall -std=c11 -I include -I src -I bin

all:liblist.so libQOL.so libcal.so libParser.so

parser:libcal.so libQOL.so

list: liblist.so

libQOL.so:QOL.o
	$(CC) -shared -o bin/libQOL.so bin/QOL.o  
QOL.o:
	$(CC) $(CFLAGS) -c src/QOL.c -fpic -o bin/QOL.o
libcal.so:CalendarParser.o
	$(CC) -shared -o bin/libcal.so bin/CalendarParser.o
CalendarParser.o:
	$(CC) $(CFLAGS) -c src/CalendarParser.c -fpic -o bin/CalendarParser.o
liblist.so:LinkedListAPI.o
	$(CC) -shared -o bin/liblist.so bin/LinkedListAPI.o
LinkedListAPI.o:
	$(CC) $(CFLAGS) -c src/LinkedListAPI.c -fpic -o bin/LinkedListAPI.o
libParser.so:QOL.o CalendarParser.o LinkedListAPI.o
	$(CC) $(CFLAGS) -shared bin/LinkedListAPI.o bin/CalendarParser.o bin/QOL.o -o ../libParser.so

clean:
	rm bin/*.o
	rm bin/*.so

 
