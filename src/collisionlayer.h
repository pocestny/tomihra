#ifndef __COLLISION_LAYER_H__
#define __COLLISION_LAYER_H__

#include  <SDL2/SDL.h>
#include <vector>

struct CollisionLayer {
  uint32_t tilesize,w,h;
  std::vector<uint32_t> data;

  CollisionLayer(uint32_t _tilesize, uint32_t _w, uint32_t _h);
  void clear();
  void fill( uint32_t v, SDL_Rect *rect);
  bool isCollision(uint32_t v, SDL_Rect *rect); // is there a collision without value v?
  std::vector<uint32_t> getCollisions(SDL_Rect *rect); // get all collision values
};

#endif
