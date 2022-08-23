#include "demolevel.h"

using namespace std;

DemoLevel::DemoLevel(SDL_Renderer* _renderer)
    : TerrainMap(_renderer, 24, 150, 150, 3000) {
#include "demolevel.inc"

  for (const string& s :
       {"door open",  "chest_open", "proximity", "shine", "keyshine"})
    layers[layerIDs[s]].visible = false;

  for (const string& s : {"background", "shadow", "door open",
                    "chest_open", "proximity", "shine","keyshine"})
    layers[layerIDs[s]].collision_active = false;

  addStage({"background", "door closed", "shadow", "base",
             "grass", "chest_open", "chest_closed","shine","keyshine"});
  addStage({"base_top","very-top","door open"});
}
