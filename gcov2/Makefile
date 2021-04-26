CFLAGS += -Wall -Werror -fprofile-arcs -ftest-coverage

LDFLAGS += -nostdlib
LDFLAGS += -nodefaultlibs
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,--warn-once

all: libgcov.a libgcovcall.a libgcovhandle.a libgcovlibc.a sample1 sample2

libgcov.a:
	./mklibgcov.sh

libgcovcall.a: gcovcall.c
	$(CC) -Wall -Werror -c gcovcall.c
	ar rv libgcovcall.a gcovcall.o

libgcovhandle.a: gcovhandle.c
	$(CC) -Wall -Werror -c gcovhandle.c
	ar rv libgcovhandle.a gcovhandle.o

libgcovlibc.a: gcovlibc.c
	$(CC) -Wall -Werror -c gcovlibc.c
	ar rv libgcovlibc.a gcovlibc.o

sample1: sample.c libgcov.a libgcovlibc.a
	$(CC) -o sample1 $(CFLAGS) sample.c ./libgcov.a ./libgcovlibc.a

sample2: sample.c libgcov.a libgcovlibc.a
	$(CC) -o sample2 $(CFLAGS) sample.c ./libgcov.a ./libgcovcall.a ./libgcovhandle.a

clean:
	rm -f *.o *.a *.gcno *.gcda sample1 sample2
