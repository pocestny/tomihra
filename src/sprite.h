#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

class Sprite {
 public:
  struct Frame {
    std::string charsheet;
    int x, y;
    uint32_t end_time;
  };

 protected:
  SDL_Renderer* renderer;
  SDL_Rect _rect, _collision_rect;
  std::unordered_map<std::string, SDL_Texture*> charsheets;
  std::unordered_map<std::string, std::vector<Frame>> animations;
  std::string current_animation;
  uint32_t current_animation_start;

 public:
  Sprite(SDL_Renderer* _renderer, int w, int h);
  ~Sprite();

  const std::string& animation_running() const { return current_animation; }
  SDL_Rect* const rect()  { return &_rect; }
  SDL_Rect* const collision_rect()  { return &_collision_rect; }
  int cx() const { return _rect.x + _rect.h / 2; }
  int cy() const { return _rect.y + _rect.w / 2; }
  void moveTo(int x, int y) {
    _collision_rect.x += x  - _rect.x;
    _rect.x = x;
    _collision_rect.y += y - _rect.y;
    _rect.y = y;
  }
  void addCharsheet(std::string name, const std::string& data);
  void addAnimation(std::string name, const std::vector<Frame>& anim);
  void startAnimation(std::string name);
  void render(const SDL_Rect* camera);
};

#endif
