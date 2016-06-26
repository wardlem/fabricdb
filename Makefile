OBJS = os.o mutex.o mem.o byteorder.o
TESTOBJS = test_os.o test_mutex.o test_mem.o test_byteorder.o
CC = gcc
DEBUG = -g
TEST = -DFABRICDB_TESTING
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)
TFLAGS =

clean:
	\rm -f *.o *~ runtest *.gcda *.gcno *.tmp *.gcov || true

byteorder.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/byteorder.c -o byteorder.o

mem.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/mem.c -o mem.o

mutex.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/mutex.c -o mutex.o

os.o: mem.o mutex.o
	$(CC) $(CFLAGS) $(TFLAGS) src/os.c mem.o mutex.o -o os.o

runtest: set_test_flags $(OBJS)
	$(CC) $(LFLAGS) $(TFLAGS) test/test_main.c $(OBJS) -o runtest

test: clean runtest
	./runtest

set_test_flags:
	$(eval TFLAGS += $(TEST) )

set_coverage_flags:
	$(eval TFLAGS += -fprofile-arcs -ftest-coverage)

coverage: set_coverage_flags test
	gcov $(OBJS)
	lcov --directory ./ -c -o application.info
	genhtml -o ./test_coverage -t "fabricdb test coverage" --num-spaces 4 application.info
