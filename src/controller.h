#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__
#include "connector.h"
#include "resources.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

class Camera {
 protected:
  SDL_Rect rect;

 public:
  Camera(SDL_Rect _rect) : rect{_rect} {}
  const SDL_Rect* operator()() const { return &rect; }
  void center_at(int x, int y, const SDL_Rect* screen);
  void print();
};

struct Controller {
  const int maxFPS = 60;
  const int ticks_per_frame = (1000 / maxFPS);
  const std::vector<SDL_Keycode> listened_keys{
      SDLK_x,    SDLK_q,     SDLK_UP,     SDLK_DOWN,
      SDLK_LEFT, SDLK_RIGHT, SDLK_LSHIFT, SDLK_RSHIFT};
  enum {
    KEY_x = 0,
    KEY_q,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LSHIFT,
    KEY_RSHIFT
  };
  std::vector<bool> key_pressed, key_pressed_old;
  std::string defaultFont;

  SDL_Window* sdl_window;
  SDL_Renderer* renderer;
  SDL_Texture *splash_image;

  SDL_Rect window;

  std::unordered_map<std::string, TTF_Font*> fonts;
  std::vector<std::string> font_data;

  Controller(const std::string& window_name, int width, int height);
  void loadFont(std::string name, std::string data, int size);
  SDL_Surface* prepareText(
      const std::string& text, SDL_Color color = {0, 0, 0},
      std::string font = "");          // allocates the surface
  void renderText(SDL_Surface*, int x, int y);  // frees the sufrace
  void renderStat();
  uint32_t iteration();
  void run();
  ~Controller();
};

#endif
