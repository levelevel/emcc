CPP=cpp

EMCC=emcc
EMCC2=emcc2

CFLAGS=-Wall -std=c11 -pedantic-errors -g -static -I./include
AFLAGS=-g -no-pie

CFLAGS2=$(CFLAGS) -D_emcc

HEADS=include/emcc.h include/util.h
SRCS=$(wildcard ./src/*.c) test_src/test_error.c
OBJS=$(SRCS:.c=.o)
SRCS2=$(wildcard ./src/*.c) test_src/test_error.c
#OBJS2=$(shell echo $(OBJS) | sed -e s^/src/^/obj2/^g )
OBJS2=obj2/main.o src/codegen.o src/parse.o src/parse_util.o obj2/tokenize.o src/dump.o src/util.o obj2/test_error.o

CPPHEADS=cpp/emcpp.h include/util.h
CPPSRCS=$(wildcard ./cpp/*.c) ./src/util.c
CPPSRCS2=$(wildcard ./cpp/*.c) ./src/util.c
CPPOBJS=$(CPPSRCS:.c=.o)
CPPOBJS2=$(shell echo $(CPPOBJS) | sed -e s^/cpp/^/obj2/^g -e s^/src/^/obj2/^g )

EMCPP=emcpp
EMCPP2=emcpp2
EMCCFLAGS=-g

all:$(EMCC) $(EMCC2) $(EMCPP) $(EMCPP2)

obj2/main.o:       src/main.c       $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/codegen.o:    src/codegen.c    $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/parse.o:      src/parse.c      $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/parse_util.o: src/parse_util.c $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/tokenize.o:   src/tokenize.c   $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/dump.o:       src/dump.c       $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/test_error.o: test_src/test_error.c $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@

obj2/emcpp.o:        cpp/emcpp.c        $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/cpp_parse.o:    cpp/cpp_parse.c    $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/cpp_tokenize.o: cpp/cpp_tokenize.c $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@
obj2/util.o:         src/util.c         $(EMCC)
	$(CPP)    $(CFLAGS2)   $< -o $*.cpp.c
	./$(EMCC) $(EMCCFLAGS) $*.cpp.c > $*.s
	$(CC) -c  $(CFLAGS2)   $*.s -o $@

$(EMCC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EMCC) $(OBJS) $(LDFLAGS)

$(EMCC2): $(OBJS2) $(EMCC)
	@echo $(OBJS2)
	$(CC) $(CFLAGS) -o $(EMCC2) $(OBJS2) $(LDFLAGS)

$(OBJS): $(HEADS)
$(OBJS2): $(HEADS)

$(EMCPP): $(CPPOBJS)
	$(CC) $(CFLAGS) -o $(EMCPP) $(CPPOBJS) $(LDFLAGS)

$(EMCPP2): $(CPPOBJS2) $(EMCC)
	$(CC) $(CFLAGS) -o $(EMCPP2) $(CPPOBJS2) $(LDFLAGS)

$(CPPOBJS): $(CPPHEADS)

#----------------------------------------------
TESTSRCS=test_src/expr.c test_src/extern.c
TESTEXEGCC=tmp/test_gcc
TESTEXEEMCC=tmp/test_emcc
TESTEXEEMCC2=tmp/test_emcc2

testall: test testcpp testcpp2 test2

test: $(TESTEXEGCC) $(TESTEXEEMCC)
	@echo "==== $@ ============================"
	./test.sh
test2: $(TESTEXEGCC) $(TESTEXEEMCC2)
	@echo "==== $@ ============================"
	./test.sh $(EMCC2)

$(TESTEXEGCC): $(TESTSRCS)
	$(CC) -g $(TESTSRCS) -o $(TESTEXEGCC) > tmp/expr.gcc.log 2>&1

$(TESTEXEEMCC): $(EMCC) $(TESTSRCS)
	$(CPP) $(CFLAGS2) test_src/expr.c -o tmp/expr.cpp.c
	./$(EMCC) tmp/expr.cpp.c    > tmp/expr.s   2>  tmp/expr.emcc.log
	./$(EMCC) test_src/extern.c > tmp/extern.s 2>> tmp/expr.emcc.log
	$(CC) $(AFLAGS) tmp/expr.s tmp/extern.s -o $(TESTEXEEMCC)

$(TESTEXEEMCC2): $(EMCC2) $(TESTSRCS)
	$(CPP) $(CFLAGS2) test_src/expr.c -o tmp/expr.cpp.c
	./$(EMCC2) tmp/expr.cpp.c    > tmp/expr2.s   2>  tmp/expr.emcc2.log
	./$(EMCC2) test_src/extern.c > tmp/extern2.s 2>> tmp/expr.emcc2.log
	$(CC) $(AFLAGS) tmp/expr2.s tmp/extern2.s -o $(TESTEXEEMCC2)

tester:  $(EMCC)
	./test.sh -e

testcpp: $(EMCPP)
	@echo "==== $@ ============================"
	./testcpp.sh emcpp
testcpp2: $(EMCPP2)
	@echo "==== $@ ============================"
	./testcpp.sh emcpp2

test_gdb: $(EMCC)
	$(CPP) $(CFLAGS) test_src/test_gdb.c    -o test_src/test_gdb.cpp.c
	./$(EMCC)  -g    test_src/test_gdb.cpp.c > test_src/test_gdb.emcc.s
	#./$(EMCC2) -g    test_src/test_gdb.cpp.c > test_src/test_gdb.emcc2.s
	$(CC) $(CFLAGS)    -o test_src/test_gdb_emcc  test_src/test_gdb.emcc.s
	#$(CC) $(CFLAGS)    -o test_src/test_gdb_emcc2 test_src/test_gdb.emcc2.s
	$(CC) $(CFLAGS)    -o test_src/test_gdb_gcc   test_src/test_gdb.c
	$(CC) $(CFLAGS) -S -masm=intel -o test_src/test_gdb_gcc.s test_src/test_gdb.c
	@#gdb ./test_src/test_gdb

clean:
	rm -f $(EMCC) $(EMCC2) $(EMCPP) $(EMCPP2) $(OBJS) $(OBJS2) $(CPPOBJS) $(CPPOBJS2)
	rm -f */*.s */*.cpp.c *~ tmp/*
	rm -f ./test_src/test_gdb ./test_src/test_gdb0