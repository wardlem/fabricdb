OBJS = pager.o os.o mutex.o mem.o byteorder.o ptrmap.o property.o fstring.o u8array.o u32array.o
CC = gcc
DEBUG = -g
TEST = -DFABRICDB_TESTING -o0
LFLAGS = -std=c99 -pedantic -Wall $(DEBUG)
CFLAGS = $(LFLAGS) -c $(DEBUG)
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
	$(CC) $(CFLAGS) $(TFLAGS) src/os.c -o os.o

pager.o: os.o mem.o byteorder.o
	$(CC) $(CFLAGS) $(TFLAGS) src/pager.c -o pager.o

ptrmap.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/ptrmap.c -o ptrmap.o

property.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/property.c -o property.o

fstring.o: mem.o
	$(CC) $(CFLAGS) $(TFLAGS) src/fstring.c -o fstring.o

u8array.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/u8array.c -o u8array.o

u32array.o:
	$(CC) $(CFLAGS) $(TFLAGS) src/u32array.c -o u32array.o

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

generate:
	./scripts/gen_hashmap.rb ptrmap "void*"
	./scripts/gen_dynarray.rb u8array "uint8_t"
	./scripts/gen_dynarray.rb u32array "uint32_t"
