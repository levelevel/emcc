EMCC=./emcc
EMCC2=./emcc2

CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I./include
CFLAGS2=$(CFLAGS) -D_emcc

HEADS=include/emcc.h include/util.h
SRCS=$(wildcard src/*.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)
SRCS2=$(wildcard src/[^u]*.c) test_src/test_error.c
OBJS2=$(SRCS2:.c=.o) obj2/util.o

CPPHEADS=cpp/emcpp.h include/util.h
CPPSRCS=$(wildcard cpp/*.c)
CPPSRCS2=$(wildcard cpp/*.c)
CPPOBJS=$(CPPSRCS:.c=.o) obj2/util.o
CPPOBJS2=cpp/emcpp.o cpp/cpp_parse.o obj2/cpp_tokenize.o obj2/util.o

TARGET=emcc
TARGET2=emcc2
CPPEXE=emcpp
CPPEXE2=emcpp2

all:$(TARGET) $(TARGET2) $(CPPEXE)

obj2/emcpp.o: cpp/emcpp.c
	cpp $(CFLAGS2) cpp/emcpp.c -o obj2/emcpp.cpp.c
	$(EMCC) obj2/emcpp.cpp.c > obj2/emcpp.s
	$(CC) -c $(CFLAGS2) obj2/emcpp.s -o obj2/emcpp.o
obj2/util.o: src/util.c
	cpp $(CFLAGS2) src/util.c -o obj2/util.cpp.c
	$(EMCC) obj2/util.cpp.c > obj2/util.s
	$(CC) -c $(CFLAGS2) obj2/util.s -o obj2/util.o
obj2/cpp_tokenize.o: cpp/cpp_tokenize.c
	cpp $(CFLAGS2) cpp/cpp_tokenize.c -o obj2/cpp_tokenize.cpp.c
	$(EMCC) obj2/cpp_tokenize.cpp.c > obj2/cpp_tokenize.s
	$(CC) -c $(CFLAGS2) obj2/cpp_tokenize.s -o obj2/cpp_tokenize.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(TARGET2): $(OBJS2) $(TARGET)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJS2) $(LDFLAGS)

$(OBJS): $(HEADS)
$(OBJS2): $(HEADS)

$(CPPEXE): $(CPPOBJS)
	$(CC) $(CFLAGS) -o $(CPPEXE) $(CPPOBJS) $(LDFLAGS)

$(CPPEXE2): $(CPPOBJS2) $(TARGET)
	$(CC) $(CFLAGS) -o $(CPPEXE2) $(CPPOBJS2) $(LDFLAGS)

$(CPPOBJS): $(CPPHEADS)

testall: test testcpp

test: $(TARGET) 
	./test.sh
tester:  $(TARGET)
	./test.sh -e

testcpp: $(CPPEXE)
	./testcpp.sh emcpp
testcpp2: $(CPPEXE2)
	./testcpp.sh emcpp2

clean:
	rm -f  $(TARGET) $(TARGET2) $(CPPEXE) $(CPPEXE2) $(OBJS) $(OBJS2) $(CPPOBJS) $(CPPOBJS2) *~ tmp/*
