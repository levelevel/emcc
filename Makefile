CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I.

HEADS=9cc.h
SRCS=$(wildcard *.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)

CPPSRCS=cpp/emcpp.c
CPPOBJS=$(CPPSRCS:.c=.o)

TARGET=emcc

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADS)

test: $(TARGET)
	./test.sh
tester:  $(TARGET)
	./test.sh -e

clean:
	rm -f  $(TARGET) *.o *~ tmp/*
