BINARY     := hra
SOURCES    := camera.cc controller.cc  main.cc resources.cc\
							microui.cc script.cc terrainmap.cc sprite.cc\
							ulpccharacter.cc demolevel.cc collisionlayer.cc
HEADERS    := base64.h connector.h camera.h controller.h hash.h \
							resources.h microui.h script.h terrainmap.h sprite.h\
							ulpccharacter.h verbose.h demolevel.h demolevel.inc \
							collisionlayer.h
OUTDIR     ?= build
RESOURCES  := 

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
DEBUGFLAGS := -D VERBOSE -D RENDER_RECT -D DRAW_UNPASSABLE
CXXFLAGS   := -g -std=c++20 -I $(srcdir) -O3 -D VERBOSE #${DEBUGFLAGS}
LDFLAGS    := -lSDL2 -lSDL2_image -lSDL2_ttf -lm
EMCC       := emcc
EMCCFLAGS  := -O3 -g -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2\
							-s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2\
							-I $(srcdir) -g -std=c++20 -lm #-D VERBOSE


all: $(resource_compiler) $(outbin)

$(resource_compiler):
	mkdir -p $(outdir)
	make -C $(resource_compiler_dir) OUT=../$(outdir)

$(srcdir)/resources.cc $(srcdir)/resources.h: $(resource_compiler) $(resources) $(resource_conf)
	$(resource_compiler) -d $(resdir) -f $(resource_conf) -o $(srcdir)/resources

#  linux binary

$(outbin): $(objects) $(headers) $(resources)
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
	cp $(resdir)/all-credits.csv $(webdir)

#  cleanup

clean:
	$(RM) -r $(outdir)
	$(RM) -r $(webdir)


.PHONY: all clean 

