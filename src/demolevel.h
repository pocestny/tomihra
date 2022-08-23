#ifndef __DEMOLEVEL_H__
#define __DEMOLEVEL_H__
#include "terrainmap.h"
#include "resources.h"

struct DemoLevel : public TerrainMap {
  DemoLevel (SDL_Renderer *_renderer);
};

#endif
