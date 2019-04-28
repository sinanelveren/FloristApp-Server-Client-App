CC = gcc
DB = gdb
CFLAGS = -o
DFLAGS = -g
MAIN = 111044074_main

PROGNAME = floristApp


all:
	$(CC) -std=c11  -c $(MAIN).c -lm -lrt -lpthread 
	$(CC) $(MAIN).o   -lm -lrt -lpthread  $(CFLAGS) $(PROGNAME)

debug:
	$(CC) -std=c11 $(DFLAGS) $(MAIN).c $(CFLAGS) $(PROGNAME)
	$(DB) ./$(PROGNAME)


clean:
	rm -f $(PROGNAME) *.o *.log
