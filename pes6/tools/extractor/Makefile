CROSS_COMPILE ?= aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
CFLAGS := -O2
CXXFLAGS := -O2 -std=c++17
VENDOR := third_party/pspdecrypt
OBJECTS := build/extract.o build/PrxDecrypter.o build/kirk_engine.o build/AES.o build/SHA1.o build/bn.o build/ec.o

all: pes6-extractor

pes6-extractor: $(OBJECTS)
	$(CXX) -static -o $@ $(OBJECTS)

build:
	mkdir -p build

build/extract.o: extract.cpp | build
	$(CXX) $(CXXFLAGS) -Wall -Wextra -Werror -c $< -o $@

build/PrxDecrypter.o: $(VENDOR)/PrxDecrypter.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: $(VENDOR)/libkirk/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build pes6-extractor

.PHONY: all clean
