BINARY  := hra
SOURCES := main.cc levelmap.cc
CXX     := g++
OUT     ?= build

srcdir    ?= src
resdir    ?= resources
sources   := $(addprefix $(srcdir)/,$(SOURCES))
outdir    := $(OUT)/linux
outbin    := $(outdir)/$(BINARY)
objects   := $(addprefix $(outdir)/,$(SOURCES:.cc=.o))

CXXFLAGS := -I $(outdir) -O3 -g -std=c++20 -I $(srcdir)
LDFLAGS  := -lSDL2 -lSDL2_image

resources := $(outdir)/resources.h
resconf   := $(resdir)/resources.conf
rcomp     := $(outdir)/resource_compiler
rcompdir  := resource_compiler

webdir    := $(OUT)/web
EMCC      := emcc
EMCCFLAGS :=  -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -I $(outdir) -I $(srcdir) -g -std=c++20

all: $(rcomp) $(outbin)

#  resources

$(rcomp): 
	mkdir -p $(outdir)
	make -C $(rcompdir) OUT=../$(outdir)

$(resources): $(resconf)
	$(rcomp) -d $(resdir) -f $(resconf) -o $@


#  linux binary

$(outbin): $(resources) $(objects) 
	mkdir -p $(outdir)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	cp $(outbin) .

$(outdir)/%.o: $(srcdir)/%.cc 
	$(CXX) $(CXXFLAGS) -o $@ -c $<


#  web

web: $(rcomp) $(resources) 
	mkdir -p $(webdir)
	$(EMCC) $(EMCCFLAGS) $(sources) -o $(webdir)/hra.html
	cp $(srcdir)/hra.html $(webdir)

#  cleanup

clean:
	$(RM) -r $(outdir)

clean-all:
	$(RM) -r $(out)

.PHONY: all clean clean-all 

