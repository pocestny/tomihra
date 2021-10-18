BINARY := resource_compiler
OUT    ?= .

CXX = g++
CXXFLAGS = -O3 

outbin  := $(OUT)/$(BINARY)
sources := INIReader.cc ini.c resource_compiler.cc
headers := INIReader.h base64.h

all: $(outbin)

$(outbin): $(sources) $(headers)
	 $(CXX) $(sources) -o $(outbin)
