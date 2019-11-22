CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I. -I./include

HEADS=include/emcc.h include/util.h
SRCS=$(wildcard src/*.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)

CPPHEADS=cpp/emcpp.h include/util.h
CPPSRCS=$(wildcard cpp/*.c)
CPPOBJS=$(CPPSRCS:.c=.o) src/util.o

TARGET=emcc
CPPEXE=emcpp

all:$(TARGET) $(CPPEXE)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADS)

$(CPPEXE): $(CPPOBJS)
	$(CC) $(CFLAGS) -o $(CPPEXE) $(CPPOBJS) $(LDFLAGS)

$(CPPOBJS): $(CPPHEADS)

test: $(TARGET) 
	./test.sh
tester:  $(TARGET)
	./test.sh -e

testcpp: $(CPPEXE)
	./$(CPPEXE) test_src/test_emcpp.c

clean:
	rm -f  $(TARGET) $(CPPEXE) $(OBJS) $(CPPOBJS) *~ tmp/*
