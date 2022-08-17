#ifndef __SPRITE_H__
#define __SPRITE_H__

/*
 * basic sprite with animations
 * has bounding (rect) and collision rectangle
 * has a number of character sheets
 * has a number of animations: each is a sequence of frames and endtimes
 * one animation is always running
 * calling render advances current animation, if it ends, emits
 * "animatione_ended" signal
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

struct Sprite {
  struct Frame {
    std::string charsheet;
    int x, y;
    uint32_t end_time;
  };

  uint32_t id;
  SDL_Renderer* renderer;
  SDL_Rect rect, collision_rect;
  std::unordered_map<std::string, SDL_Texture*> charsheets;
  std::unordered_map<std::string, std::vector<Frame>> animations;
  std::string current_animation;
  uint32_t current_animation_start;  // time when animation started
  int cxo, cyo;                      // offset to center

  Sprite(uint32_t _id, SDL_Renderer* _renderer, int w, int h);
  ~Sprite();

  int cx() const { return rect.x + cxo; }
  int cy() const { return rect.y + cyo; }
  void moveTo(int x, int y) {
    collision_rect.x += x - rect.x;
    rect.x = x;
    collision_rect.y += y - rect.y;
    rect.y = y;
  }
  void addCharsheet(std::string name, const std::string& data);
  void addAnimation(std::string name, const std::vector<Frame>& anim);
  void startAnimation(std::string name);
  void render(const SDL_Rect* camera);
};

#endif
