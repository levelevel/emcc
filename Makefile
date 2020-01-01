CPP=cpp

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
CPPOBJS2=obj2/emcpp.o cpp/cpp_parse.o obj2/cpp_tokenize.o obj2/util.o

EMCPP=emcpp
EMCPP2=emcpp2
EMCCFLAGS=-g

all:$(EMCC) $(EMCC2) $(EMCPP)

obj2/emcpp.o: cpp/emcpp.c $(EMCC)
	$(CPP) $(CFLAGS2) cpp/emcpp.c -o obj2/emcpp.cpp.c
	$(EMCC) $(EMCCFLAGS) obj2/emcpp.cpp.c > obj2/emcpp.s
	$(CC) -c $(CFLAGS2) obj2/emcpp.s -o obj2/emcpp.o
obj2/cpp_parse.o: cpp/cpp_parse.c $(EMCC)
	$(CPP) $(CFLAGS2) cpp/cpp_parse.c -o obj2/cpp_parse.cpp.c
	$(EMCC) $(EMCCFLAGS) obj2/cpp_parse.cpp.c > obj2/cpp_parse.s
	$(CC) -c $(CFLAGS2) obj2/cpp_parse.s -o obj2/cpp_parse.o
obj2/cpp_tokenize.o: cpp/cpp_tokenize.c $(EMCC)
	$(CPP) $(CFLAGS2) cpp/cpp_tokenize.c -o obj2/cpp_tokenize.cpp.c
	$(EMCC) $(EMCCFLAGS) obj2/cpp_tokenize.cpp.c > obj2/cpp_tokenize.s
	$(CC) -c $(CFLAGS2) obj2/cpp_tokenize.s -o obj2/cpp_tokenize.o
obj2/util.o: src/util.c $(EMCC)
	$(CPP) $(CFLAGS2) src/util.c -o obj2/util.cpp.c
	$(EMCC) $(EMCCFLAGS) obj2/util.cpp.c > obj2/util.s
	$(CC) -c $(CFLAGS2) obj2/util.s -o obj2/util.o

$(EMCC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EMCC) $(OBJS) $(LDFLAGS)

$(EMCC2): $(OBJS2) $(EMCC)
	$(CC) $(CFLAGS) -o $(EMCC2) $(OBJS2) $(LDFLAGS)

$(OBJS): $(HEADS)
$(OBJS2): $(HEADS)

$(EMCPP): $(CPPOBJS)
	$(CC) $(CFLAGS) -o $(EMCPP) $(CPPOBJS) $(LDFLAGS)

$(EMCPP2): $(CPPOBJS2) $(EMCC)
	$(CC) $(CFLAGS) -o $(EMCPP2) $(CPPOBJS2) $(LDFLAGS)

$(CPPOBJS): $(CPPHEADS)

testall: test testcpp testcpp2

test: $(EMCC) 
	./test.sh
tester:  $(EMCC)
	./test.sh -e

testcpp: $(EMCPP)
	./testcpp.sh emcpp
testcpp2: $(EMCPP2)
	./testcpp.sh emcpp2

test_gdb: $(EMCC)
	$(CPP) $(CFLAGS) test_src/test_gdb.c -o test_src/test_gdb.cpp.c
	$(EMCC) test_src/test_gdb.cpp.c > test_src/test_gdb.s
	$(CC) $(CFLAGS) -o test_src/test_gdb test_src/test_gdb.s
	$(CC) $(CFLAGS) -o test_src/test_gdb0 test_src/test_gdb.c
	$(CC) $(CFLAGS) -S -o test_src/test_gdb0.s test_src/test_gdb.c
	gdb ./test_src/test_gdb

clean:
	rm -f $(EMCC) $(EMCC2) $(EMCPP) $(EMCPP2) $(OBJS) $(OBJS2) $(CPPOBJS) $(CPPOBJS2)
	rm -f */*.s */*.cpp.c *~ tmp/*
	rm -f ./test_src/test_gdb ./test_src/test_gdb0