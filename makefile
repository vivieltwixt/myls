CC=gcc
CFLAGS= -g
RM=/bin/rm -f


myls: myls.c 
	$(CC) $(CFLAGS) -o $@ myls.c dllist.c  jval.c


clean:
	$(RM) myls *~
