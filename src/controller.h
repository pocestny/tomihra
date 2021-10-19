#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cassert>
#include <iostream>
#include "connector.h"

using namespace std;

class Camera {
 protected:
  SDL_Rect rect;

 public:
  Camera(SDL_Rect _rect) : rect{_rect} {
    // print();
  }
  const SDL_Rect* operator()() const { return &rect; }
  void center_at(int x, int y, const SDL_Rect* screen) {
    // cout<<"Camera::center_at("<<x<<","<<y<<",["<<screen->x<<","<<screen->y<<","<<screen->w<<","<<screen->h<<"])\n";
    rect.x = x - rect.w / 2;
    if (rect.x < screen->x) rect.x = screen->x;
    if (rect.x + rect.w > screen->x + screen->w)
      rect.x = screen->x + screen->w - rect.w;
    rect.y = y - rect.h / 2;
    if (rect.y < screen->y) rect.y = screen->y;
    if (rect.y + rect.h > screen->y + screen->h)
      rect.y = screen->y + screen->h - rect.h;
  }
  void print() {
    cout << "Camera [" << rect.x << "," << rect.y << "," << rect.w << ","
         << rect.h << "]\n";
  }
};

struct Controller {
  int maxFPS = 60;
  int ticks_per_frame = (1000 / maxFPS);
  const vector<SDL_Keycode> listened_keys{SDLK_x, SDLK_UP, SDLK_DOWN, SDLK_LEFT,
                                          SDLK_RIGHT};
  enum { KEY_x = 0, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT };
  vector<bool> key_pressed, key_pressed_old;

  SDL_Window* sdl_window;
  SDL_Renderer* renderer;
  SDL_Rect window;

  Controller(const string &window_name, int width, int height)
      : window{.x = 0, .y = 0, .w = width, .h = height} {
    key_pressed.resize(listened_keys.size(), false);
    key_pressed_old.resize(listened_keys.size(), false);
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
    assert(sdl_window = SDL_CreateWindow(window_name.data(), SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED, window.w,
                                         window.h, SDL_WINDOW_SHOWN));
    assert(renderer =
               SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED));
    assert(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG);
  }

  uint32_t iteration() {
    SDL_Event e;
    uint32_t start = SDL_GetTicks();
    key_pressed_old = key_pressed;
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_KEYDOWN) {
        for (int i = 0; i < listened_keys.size(); i++)
          if (e.key.keysym.sym == listened_keys[i]) key_pressed[i] = true;
      } else if (e.type == SDL_KEYUP) {
        for (int i = 0; i < listened_keys.size(); i++)
          if (e.key.keysym.sym == listened_keys[i]) key_pressed[i] = false;
      }
    }
    SDL_RenderClear(renderer);
    Connector<int>::emit(this, "frame", 0);
    SDL_RenderPresent(renderer);
    uint32_t end = SDL_GetTicks();
    return end - start;
  }

  void run() {
    bool quit = false;
    while (!quit) {
      uint32_t time = iteration();
      if (key_pressed[KEY_x]) return;
      if (time < ticks_per_frame) SDL_Delay(ticks_per_frame - time);
    }
  }

  ~Controller() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdl_window);
    IMG_Quit();
    SDL_Quit();
  }
};

#endif
