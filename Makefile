CFLAGS=-Wall -std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./9cc -test
	./test.sh
tester: 9cc
	./test.sh -e

clean:
	rm -f 9cc *.o *~ tmp/*
