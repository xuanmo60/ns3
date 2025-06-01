CC = g++
CFLAGS = -std=c++23 -Wall

all: mkdir ping_caller printer

mkdir: clean
	mkdir ./build

ping_caller: ping_caller.cc
	$(CC) $(CFLAGS) -o ./build/$@ $<

printer: printer.cc
	$(CC) $(CFLAGS) -o ./build/$@ $<

clean:
	rm -rf ./build
