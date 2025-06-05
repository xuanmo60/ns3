# Makefile

CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2

# Executable names
CALLER := pingCaller
HANDLER := pingHandler

# FIFO path
FIFO_PATH := /tmp/ping_fifo

.PHONY: all clean

all: build $(CALLER) $(HANDLER)

build:
	mkdir ./build

$(CALLER): pingCaller.cc
	$(CXX) $(CXXFLAGS) -o ./build/$(CALLER) pingCaller.cc

$(HANDLER): pingHandler.cc
	$(CXX) $(CXXFLAGS) -o ./build/$(HANDLER) pingHandler.cc

clean:
	-rm -f ./build
	-rm -f $(FIFO_PATH)
