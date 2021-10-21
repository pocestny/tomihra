BINARY     := hra
SOURCES    := controller.cc levelmap.cc main.cc sprite.cc resources.cc\
							microui.cc sample_level.cc
HEADERS    := base64.h connector.h controller.h hash.h levelmap.h sprite.h\
							resources.h microui.h sample_level.h 
OUTDIR     ?= build
RESOURCES  := tilemap.png tilesheet.png player_sheet.png

srcdir    ?= src
resdir    ?= resources
sources   := $(addprefix $(srcdir)/,$(SOURCES))
outdir    := $(OUTDIR)/linux
webdir    := $(OUTDIR)/web
outbin    := $(outdir)/$(BINARY)
objects   := $(addprefix $(outdir)/,$(SOURCES:.cc=.o))
resources := $(addprefix $(resdir)/,$(RESOURCES))
headers   := $(addprefix $(srcdir)/,$(HEADERS))

resource_compiler     := $(outdir)/resource_compiler
resource_compiler_dir := resource_compiler
resource_conf         := $(resdir)/resources.conf

CXX        := clang++
CXXFLAGS   := -O3 -g -std=c++20 -I $(srcdir)
LDFLAGS    := -lSDL2 -lSDL2_image -lSDL2_ttf
EMCC       := emcc
EMCCFLAGS  := -O3 -g -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2\
							-s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2\
							-I $(srcdir) -g -std=c++20


all: $(resource_compiler) $(outbin)

$(resource_compiler):
	mkdir -p $(outdir)
	make -C $(resource_compiler_dir) OUT=../$(outdir)

$(srcdir)/resources.cc $(srcdir)/resources.h: $(resource_compiler) $(resources) $(resource_conf)
	$(resource_compiler) -d $(resdir) -f $(resource_conf) -o $(srcdir)/resources

#  linux binary

$(outbin): $(objects) $(headers) 
	mkdir -p $(outdir)
	$(CXX) $(CXXFLAGS) -o $@ $(objects) $(LDFLAGS)
	cp $(outbin) .

$(outdir)/%.o: $(srcdir)/%.cc $(headers) 
	$(CXX) $(CXXFLAGS) -o $@ -c $<

#  web

web: $(sources) $(headers)
	mkdir -p $(webdir)
	$(EMCC) $(EMCCFLAGS) $(sources) -o $(webdir)/hra.html
	cp $(srcdir)/hra.html $(webdir)

#  cleanup

clean:
	$(RM) -r $(outdir)

clean-all:
	$(RM) -r $(out)

.PHONY: all clean clean-all 

