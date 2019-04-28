SRC = util.c 
HEAD = util.h

all: 9cc runtest
9cc: 9cc.c ${SRC} ${HEAD}
runtest: runtest.c ${SRC} ${HEAD}

test: 9cc runtest
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*
