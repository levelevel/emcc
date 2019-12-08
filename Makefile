EMCC=./emcc
EMCC2=./emcc2

CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I./include
CFLAGS2=$(CFLAGS) -D_emcc

HEADS=include/emcc.h include/util.h
SRCS=$(wildcard src/*.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)
SRCS2=$(wildcard src/[^u]*.c) test_src/test_error.c
OBJS2=obj2/util.o $(SRCS2:.c=.o)

CPPHEADS=cpp/emcpp.h include/util.h
CPPSRCS=$(wildcard cpp/*.c)
CPPOBJS=$(CPPSRCS:.c=.o) obj2/util.o

TARGET=emcc
TARGET2=emcc2
CPPEXE=emcpp

all:$(TARGET) $(TARGET2) $(CPPEXE)

obj/util.o: src/util.c
	cpp $(CFLAGS2) src/util.c -o obj2/util.cpp.c
	$(EMCC) obj2/util.cpp.c > obj2/util.s
	$(CC) -c $(CFLAGS2) obj2/util.s -o obj2/util.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJS2) $(LDFLAGS)

$(OBJS): $(HEADS)
$(OBJS2): $(HEADS)

$(CPPEXE): $(CPPOBJS)
	$(CC) $(CFLAGS) -o $(CPPEXE) $(CPPOBJS) $(LDFLAGS)

$(CPPOBJS): $(CPPHEADS)

testall: test testcpp

test: $(TARGET) 
	./test.sh
tester:  $(TARGET)
	./test.sh -e

testcpp: $(CPPEXE)
	./testcpp.sh

clean:
	rm -f  $(TARGET) $(CPPEXE) $(OBJS) $(CPPOBJS) *~ tmp/*
