#include "connector.h"
#include "levelmap.h"
#include "resources.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <iostream>

using namespace std;

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

  Controller() : window{.x = 0, .y = 0, .w = 1200, .h = 900} {
    key_pressed.resize(listened_keys.size(), false);
    key_pressed_old.resize(listened_keys.size(), false);
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
    assert(sdl_window = SDL_CreateWindow("hra", SDL_WINDOWPOS_UNDEFINED,
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
    Connector<int>::emit(this, "render", 0);
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

class Sprite {
 protected:
  SDL_Renderer* renderer;
  SDL_Rect _rect;
  SDL_Texture* tx;

 public:
  Sprite(SDL_Renderer* _renderer, int w, int h)
      : renderer{_renderer},
        _rect{.x = 0, .y = 0, .w = w, .h = h},
        tx{nullptr} {}
  ~Sprite() {
    if (tx) SDL_DestroyTexture(tx);
  }
  const SDL_Rect* rect() const { return &_rect; }
  int cx() const { return _rect.x + _rect.h / 2; }
  int cy() const { return _rect.y + _rect.w / 2; }
  void moveTo(int x, int y) {
    _rect.x = x;
    _rect.y = y;
  }
  void addImage(const std::string& data) {
    SDL_RWops* tmp;
    SDL_Surface* surf;
    assert(tmp = SDL_RWFromConstMem((const void*)data.data(), data.size()));
    assert(surf = IMG_Load_RW(tmp, 1));
    assert(surf->w == _rect.w);
    assert(surf->h == _rect.h);
    assert(tx = SDL_CreateTextureFromSurface(renderer, surf));
    SDL_FreeSurface(surf);
  }
  void render(const SDL_Rect* camera) {
    SDL_Rect src, dst;
    if (SDL_IntersectRect(&_rect, camera, &src) == SDL_FALSE) return;
    dst = src;
    src.x -= _rect.x;
    src.y -= _rect.y;
    dst.x -= camera->x;
    dst.y -= camera->y;
    SDL_RenderCopy(renderer, tx, &src, &dst);
  }
};

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

#ifdef __EMSCRIPTEN__
Controller* ext_c;
EM_BOOL iteration(double time, void* userData) {
  ext_c->iteration();
  return EM_TRUE;
}
#endif

int main(int argc, char** argv) {
  Controller* c = new Controller();
#ifdef __EMSCRIPTEN__
  ext_c = c;
#endif
  LevelMap* m = new LevelMap(c->renderer, 32, 40, 40);
  Camera* camera = new Camera(c->window);
  Sprite* p = new Sprite(c->renderer, 48, 48);

  m->addTiles({{0xff, "sheet", 0, 0, false},
               {0xffffff, "sheet", 0, 32, true},
               {0xff00ffff, "sheet", 0, 64, true}});
  m->addTileSheet("sheet", Level1Resources::sheet());
  m->addTileMap(0, Level1Resources::mapa());

  p->addImage(Level1Resources::sprite());

  Connector<int>::connect(c, "render", [m, c, p, camera](int) {
    // cout<<"2: ";
    // m->printScreen();
    // camera->print();
    int dx = 0, dy = 0;
    int r = 5;
    if (c->key_pressed[Controller::KEY_UP]) dy = -r;
    if (c->key_pressed[Controller::KEY_DOWN]) dy = r;
    if (c->key_pressed[Controller::KEY_LEFT]) dx = -r;
    if (c->key_pressed[Controller::KEY_RIGHT]) dx = r;
    SDL_Rect npos = *(p->rect());
    npos.x += dx;
    npos.y += dy;
    if (m->accessible(&npos)) p->moveTo(npos.x, npos.y);
    camera->center_at(p->cx(), p->cy(), m->screen());
    m->render(0, (*camera)());
    p->render((*camera)());
  });
#ifdef __EMSCRIPTEN__
  // cout<<"1: ";
  // m->printScreen();
  emscripten_request_animation_frame_loop(iteration, 0);
#else
  c->run();
#endif
}
