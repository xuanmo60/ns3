# Makefile

CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2

# Executable names
CALLER := pingCaller
HANDLER := pingHandler

# FIFO path
FIFO_PATH := /tmp/ping_fifo

.PHONY: all clean

all: $(CALLER) $(HANDLER)

$(CALLER): pingCaller.cc
	$(CXX) $(CXXFLAGS) -o $(CALLER) pingCaller.cc

$(HANDLER): pingHandler.cc
	$(CXX) $(CXXFLAGS) -o $(HANDLER) pingHandler.cc

clean:
	-rm -f $(CALLER) $(HANDLER)
	-rm -f $(FIFO_PATH)
