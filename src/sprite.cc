#include "sprite.h"
#include "connector.h"
#include <iostream>

using namespace std;

Sprite::Sprite(SDL_Renderer* _renderer, int w, int h)
    : renderer{_renderer}, _rect{.x = 0, .y = 0, .w = w, .h = h} {}

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
    cout<<"cannot start "<<current_animation<<endl;
    return;
  }
  int f = 0;
  uint32_t time = SDL_GetTicks() - current_animation_start;
  vector<Frame>& anim = animations[current_animation];
  while (f < anim.size() && anim[f].end_time < time) f++;
  //cout<<current_animation_start<<" "<<SDL_GetTicks()<<"  :  "<<time<<"  "<<f<<endl;
  if (f == anim.size()) {
    current_animation = "";
    Connector<string>::emit(this,"animation_ended", current_animation);
    return;
  }
  assert(charsheets.count(anim[f].charsheet) > 0);

  SDL_Rect src, dst;
  if (SDL_IntersectRect(&_rect, camera, &src) == SDL_FALSE) return;
  dst = src;
  src.x += anim[f].x;
  src.y += anim[f].y;
  src.x -= _rect.x;
  src.y -= _rect.y;
  dst.x -= camera->x;
  dst.y -= camera->y;
  SDL_RenderCopy(renderer, charsheets[anim[f].charsheet], &src, &dst);
}
