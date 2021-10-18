#include "connector.h"
#include "defines.h"
#include "levelmap.h"
#include "resources.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>
#include <iostream>

using namespace std;

struct Controller {
  SDL_Window* window;
  SDL_Renderer* renderer;

  Controller() {
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
    assert(window = SDL_CreateWindow("hra", SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED, window_width,
                                     window_height, SDL_WINDOW_SHOWN));
    assert(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    assert(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG);
  }

  void run() {
    SDL_Event e;
    bool quit = false;
    while (!quit) {
      uint32_t start = SDL_GetTicks();
      Connector<int>::emit(this, "frame", 0);
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_KEYDOWN) {
          switch (e.key.keysym.sym) {
            case SDLK_x:
              quit = true;
              break;
            case SDLK_UP:
              Connector<int>::emit(this, "key", 1);
              break;
            case SDLK_DOWN:
              Connector<int>::emit(this, "key", 2);
              break;
            case SDLK_LEFT:
              Connector<int>::emit(this, "key", 3);
              break;
            case SDLK_RIGHT:
              Connector<int>::emit(this, "key", 4);
              break;
          }
        }
      }
      SDL_RenderClear(renderer);
      Connector<int>::emit(this, "render", 0);
      SDL_RenderPresent(renderer);
      uint32_t end = SDL_GetTicks();
      if (end - start < ticks_per_frame)
        SDL_Delay(ticks_per_frame - end + start);
    }
  }

  ~Controller() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
  }
};

int main(int argc, char** argv) {
  Controller c;
  SDL_Rect hracpos = {.x = 10, .y = 10, .w = 40, .h = 40};
  SDL_Rect camera = {.x = 0, .y = 0, .w = window_width, .h = window_height};
  int dx = 0, dy = 0, r = 5;

  LevelMap m(c.renderer, 32, 40, 40, 128);
  m.addTiles({{0xff, "sheet", 0, 0, false},
              {0xffffff, "sheet", 0, 32, true},
              {0xff00ffff, "sheet", 0, 64}});
  m.addTileSheet("sheet", Level1Resources::sheet());
  m.addTileMap(0, Level1Resources::mapa());

  Connector<int>::connect(&c, "frame", [&](int) { dx = dy = 0; });
  Connector<int>::connect(&c, "key", [&](int x) {
    switch (x) {
      case 1:
        dy = -r;
        break;
      case 2:
        dy = r;
        break;
      case 3:
        dx = -r;
        break;
      case 4:
        dx = r;
        break;
    }
  });
  Connector<int>::connect(&c, "render", [&](int) {
    SDL_Rect npos = hracpos;
    npos.x += dx;
    npos.y += dy;
    if (npos.x > 0 && npos.y > 0 && npos.x + npos.w < m.px_width() &&
        npos.y + npos.h < m.px_height())
      hracpos = npos;
    camera.x = hracpos.x - window_width / 2;
    if (camera.x < 0) camera.x = 0;
    if (camera.x + camera.w > m.px_width()) camera.x = m.px_width() - camera.w;

    camera.y = hracpos.y - window_height / 2;
    if (camera.y < 0) camera.y = 0;
    if (camera.y + camera.h > m.px_height())
      camera.y = m.px_height() - camera.h;

    m.render(0, &camera);
  });
  c.run();
}
