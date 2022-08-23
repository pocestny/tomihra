#include "items.h"
#include "verbose.h"

using namespace std;
void Items::render(const SDL_Rect *camera) {
  for (auto &it : itemData) {
    Item &t = it.second;
    if (t.ltype == Item::Map) {
      SDL_Rect rect{
          .x = (int)t.loc.mloc.px, .y = (int)t.loc.mloc.py, .w = 32, .h = 32},
          src, dst;

      if (SDL_IntersectRect(&rect, camera, &src) == SDL_FALSE) continue;
      dst = src;
      src.x += 0;  // offset in iconsheet
      src.y += 0;
      src.x -= rect.x;
      src.y -= rect.y;
      dst.x -= camera->x;
      dst.y -= camera->y;
      SDL_RenderCopy(renderer, iconsheet, &src, &dst);
    }
  }
}

void Items::renderItem(uint32_t id, const SDL_Rect *camera, pair<int, int> ofs) {
  // slot in inventory has size 48x48
  SDL_Rect rect{
      .x = ofs.first, .y = ofs.second, .w = 48, .h = 48},
      dst;

  if (SDL_IntersectRect(&rect, camera, &dst) == SDL_FALSE) return;
  SDL_RenderCopy(renderer, iconsheet, &itemData[id].iconRect, &dst);
}
