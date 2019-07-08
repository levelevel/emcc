CFLAGS=-Wall -std=c11 -pedantic-errors -g -static
SRCS=$(wildcard *.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) $(CFLAGS) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh
tester: 9cc
	./test.sh -e

clean:
	rm -f 9cc *.o *~ tmp/*
