#include "demolevel.h"

using namespace std;

DemoLevel::DemoLevel (SDL_Renderer *_renderer) : TerrainMap (_renderer, 24, 150, 150, 3000){
  #include "demolevel.inc"

 for (int i:{0,1,5,6,7,8,9})
  collision_active_layers.erase(i);
 
   for (int i:{5,7,9})
  visible_layers.erase(i);
}

void DemoLevel::render_stage(int stage, const SDL_Rect *camera){
  static const vector<vector<int>> stage_layers{{0,1,2,3,4,5,9},{6,8}};
  for (auto &l:stage_layers[stage])if (visible_layers.count(l))
    render(l,camera);
}
