BINARY  := hra
SOURCES := main.cc levelmap.cc
CXX     := g++
OUT     ?= build

srcdir    ?= src
resdir    ?= resources
sources   := $(srcdir)/$(SOURCES)
outdir    := $(OUT)
outbin    := $(outdir)/$(BINARY)
objects   := $(addprefix $(outdir)/,$(SOURCES:.cc=.o))

CXXFLAGS := -I $(outdir) -O3 -g -std=c++20 -I $(srcdir)
LDFLAGS  := -lSDL2 -lSDL2_image

resources := $(outdir)/resources.h
resconf   := $(resdir)/resources.conf
rcomp     := $(outdir)/resource_compiler
rcompdir  := resource_compiler

all: $(rcomp) $(outbin)

$(rcomp): 
	mkdir -p $(outdir)
	make -C $(rcompdir) OUT=../$(outdir)

$(resources): $(resconf)
	$(rcomp) -d $(resdir) -f $(resconf) -o $@

$(outbin): $(resources) $(objects) 
	mkdir -p $(outdir)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	cp $(outbin) .

$(outdir)/%.o: $(srcdir)/%.cc 
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) -r $(outdir)

clean-all:
	$(RM) -r $(out)

.PHONY: all clean clean-all 

