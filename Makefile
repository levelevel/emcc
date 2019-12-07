EMCC=./emcc
EMCC2=./emcc2

CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I./include
CFLAGS2=$(CFLAGS) -D_emcc

HEADS=include/emcc.h include/util.h
SRCS=$(wildcard src/*.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)
OBJS2=obj/util.o

CPPHEADS=cpp/emcpp.h include/util.h
CPPSRCS=$(wildcard cpp/*.c)
CPPOBJS=$(CPPSRCS:.c=.o) src/util.o

TARGET=emcc
TARGET2=emcc2
CPPEXE=emcpp

all:$(TARGET) $(TARGET2) $(CPPEXE)

obj/util.o: src/util.c
	cpp $(CFLAGS2) src/util.c -o obj/util.cpp.c
	$(EMCC) obj/util.cpp.c > obj/util.s
	$(CC) $(CFLAGS2) obj/util.s -o obj/util.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(TARGET2): $(OBJS2)

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
