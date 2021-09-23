    CC     = gcc -g -std=c11 -std=gnu99
    CFLAGS = -Wall
    LFLAGS = -lm

      PROG = testafila
      OBJS = queue.o

.PHONY: limpa faxina clean purge all

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $<

$(PROG) : % :  $(OBJS) %.o
	$(CC) -o $@ $^ $(LFLAGS)

limpa clean:
	@rm -f *~ *.bak

faxina purge:   limpa
	@rm -f *.o core a.out
	@rm -f $(PROG)