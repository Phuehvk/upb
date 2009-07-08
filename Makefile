
.PHONY: all clean
CC=gcc
CXX=g++
CFLAGS=-std=c99
CPPFLAGS=-O3 -Wall -Wextra -pedantic -g -DUPB_UNALIGNED_READS_OK -fomit-frame-pointer
OBJ=upb_parse.o upb_table.o upb_msg.o upb_enum.o upb_context.o upb_string.o descriptor.o
all: $(OBJ) test_table tests upbc
clean:
	rm -f *.o test_table tests

libupb.a: $(OBJ)
	ar rcs libupb.a $(OBJ)
test_table: libupb.a
upbc: libupb.a
benchmark: libupb.a -lm google_speed.pb.o benchmark.o
	g++ $(CPPFLAGS) benchmark.cc libupb.a google_speed.pb.o   -o benchmark -lm -lprotobuf -lpthread

-include deps
deps: *.c *.h
	gcc -MM *.c > deps
