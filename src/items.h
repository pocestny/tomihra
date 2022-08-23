#ifndef __ITEMS_H__
#define __ITEMS_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <unordered_map>
#include "camera.h"

struct ItemLocationOnMap {
  uint32_t px, py;
};


struct Item {
  uint32_t id;
  enum { Map, Inventory, Hidden } ltype;
  union {
    ItemLocationOnMap mloc;
  } loc;
  SDL_Rect iconRect;
};

struct Items {
  SDL_Renderer* renderer;
  SDL_Texture *iconsheet;
  std::unordered_map<uint32_t,Item> itemData;

  Items(SDL_Renderer *_r):renderer{_r}{}
  Item &operator[](uint32_t id){return itemData[id];}
  void render(const SDL_Rect *camera);  
  void renderItem(uint32_t id,const SDL_Rect *camera, std::pair<int,int> offs);
};

#endif
