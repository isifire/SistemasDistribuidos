CFLAGS=-Wall
LDFLAGS=-pthread

OBJETIVOS=filosofos maestro
all: $(OBJETIVOS)

%: %.o
	$(CC) -o $@ $< $(LDFLAGS)
%: %.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)
clean:
	-rm *~ *.o

cleanall: clean
	-rm $(OBJETIVOS)