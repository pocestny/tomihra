#include "collisionlayer.h"
using namespace std;

CollisionLayer::CollisionLayer(uint32_t _tilesize, uint32_t _w, uint32_t _h)
    : tilesize{_tilesize}, w{_w}, h{_h} {
  clear();
}

void CollisionLayer::clear() {
  data.clear();
  data.resize(w * h);
}

void CollisionLayer::fill(uint32_t v, SDL_Rect *rect) {
  for (int x = (rect->x / tilesize)-1; x < (rect->x + rect->w) / tilesize+1; x++)
    for (int y = (rect->y / tilesize)-1; y < (rect->y + rect->h) / tilesize+1;
         y++) {
      int i = y * w + x;
      if (i < 0 || i > data.size()) continue;
      data[i].push_back( v);
    }
}

bool CollisionLayer::isCollision(uint32_t v, SDL_Rect *rect) {
  for (int x = (rect->x / tilesize); x < (rect->x + rect->w) / tilesize; x++)
    for (int y = (rect->y / tilesize); y < (rect->y + rect->h) / tilesize;
         y++) {
      int i = y * w + x;
      if (i < 0 || i > data.size()) continue;
      for(uint32_t w : data[i])
      if (w != v) return true;
    }
  return false;
}

vector<uint32_t> CollisionLayer::getCollisions(SDL_Rect *rect) {
  vector<uint32_t> res;
  for (int x = (rect->x / tilesize); x < (rect->x + rect->w) / tilesize; x++)
    for (int y = (rect->y / tilesize); y < (rect->y + rect->h) / tilesize;
         y++) {
      int i = y * w + x;
      if (i < 0 || i >data.size()) continue;
      for(uint32_t w : data[i])
      res.push_back(w);
    }
  return res;
}
