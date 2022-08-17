#include "sprite.h"

#include "connector.h"
#include "verbose.h"

using namespace std;

Sprite::Sprite(uint32_t _id, SDL_Renderer* _renderer, int w, int h)
    : id{_id},
      renderer{_renderer},
      rect{.x = 0, .y = 0, .w = w, .h = h},
      collision_rect{rect},
      cxo{h / 2},
      cyo{w / 2} {}

Sprite::~Sprite() {
  for (auto x : charsheets) SDL_DestroyTexture(x.second);
}

void Sprite::addCharsheet(string name, const string& data) {
  SDL_RWops* tmp;
  SDL_Surface* surf;
  assert(tmp = SDL_RWFromConstMem((const void*)data.data(), data.size()));
  assert(surf = IMG_Load_RW(tmp, 1));
  if (charsheets.count(name) > 0) SDL_DestroyTexture(charsheets[name]);
  assert(charsheets[name] = SDL_CreateTextureFromSurface(renderer, surf));
  SDL_FreeSurface(surf);
}

void Sprite::addAnimation(string name, const vector<Frame>& anim) {
  animations[name] = anim;
}

void Sprite::startAnimation(string name) {
  assert(animations.count(name) > 0);
  current_animation = name;
  current_animation_start = SDL_GetTicks();
}

void Sprite::render(const SDL_Rect* camera) {
  if (animations.count(current_animation) == 0) {
    cerr << "cannot start " << current_animation << endl;
    return;
  }
  int f = 0;
  uint32_t time = SDL_GetTicks() - current_animation_start;
  vector<Frame>& anim = animations[current_animation];
  while (f < anim.size() && anim[f].end_time < time) f++;
  if (f == anim.size()) {
    current_animation = "";
    Connector<string>::emit(this, "animation_ended", current_animation);
    render(camera);
    return;
  }
  assert(charsheets.count(anim[f].charsheet) > 0);

  SDL_Rect src, dst;
  if (SDL_IntersectRect(&rect, camera, &src) == SDL_FALSE) return;
  dst = src;
  src.x += anim[f].x;
  src.y += anim[f].y;
  src.x -= rect.x;
  src.y -= rect.y;
  dst.x -= camera->x;
  dst.y -= camera->y;
  SDL_RenderCopy(renderer, charsheets[anim[f].charsheet], &src, &dst);

#ifdef RENDER_RECT  
  {
    SDL_RenderDrawRect(renderer, &dst);
    SDL_IntersectRect(&collision_rect, camera, &dst);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    dst.x -= camera->x;
    dst.y -= camera->y;
    SDL_RenderDrawRect(renderer, &dst);
  }
#endif  
}
