CC = gcc
COPTS = -O3 -Wall
CINCLUDE = -I./
CFLAGS = $(COPTS) $(CINCLUDE) $(INLINE) $(OPTIONS)

HDRS = config.h bmp180.h
SRCS = bmp180.c
OBJS = $(SRCS:.c=.o)

TEST_SRCS = test_driver.c bmp180_test.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

BINS = test_driver

test_driver: $(TEST_OBJS) $(OBJS) $(HDRS)
	$(CC) $(TEST_OBJS) $(OBJS) -o $@

default: all

all: $(HDRS) $(BINS)

Makefile.depend:
	touch Makefile.depend

depend: Makefile.depend
	makedepend -fMakefile.depend

.c.o: $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

test: test_driver
	./test_driver

clean: depend
	/bin/rm -rf $(BINS) $(OBJS) $(TEST_OBJS)

include Makefile.depend
