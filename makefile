CC=gcc
CFLAGS= -g
RM=/bin/rm -f


myls: myls.c 
	$(CC) $(CFLAGS) -o $@ myls.c 


clean:
	$(RM) myls *~
