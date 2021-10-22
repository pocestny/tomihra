#include "controller.h"
#include "sample_level.h"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <iostream>

using namespace std;

typedef enum { S_MENU = 0, S_LEVEL1, S_END } state_t;
Controller *controller;

#ifdef __EMSCRIPTEN__
EM_BOOL iteration(double time, void *userData) {
  controller->iteration();
  return EM_TRUE;
}
#endif

struct MainMenu {
  SDL_Texture *splash_image;
  Controller *c;

  MainMenu(Controller *_c) : c{_c} {
    c->microUi.on = true;
    SDL_RWops *rw;
    SDL_Surface *srf;
    string data = Splash::image();
    assert(rw = SDL_RWFromConstMem((const void *)data.data(), data.size()));
    assert(srf = IMG_Load_RW(rw, 1));
    splash_image = SDL_CreateTextureFromSurface(c->renderer, srf);
    SDL_FreeSurface(srf);
  }

  ~MainMenu() { SDL_DestroyTexture(splash_image); }

  void render() {
    SDL_RenderCopy(c->renderer, splash_image, NULL, NULL);
    auto ctx = controller->mu_ctx();
    mu_Style s, *olds;
    olds = ctx->style;
    s = *olds;
    s.font = (void *)"large";
    s.padding = 30;
    s.colors[MU_COLOR_BUTTON] = {50, 200, 200, 100};
    s.colors[MU_COLOR_BUTTONHOVER] = {50, 200, 200, 150};
    s.colors[MU_COLOR_BUTTONFOCUS] = {50, 200, 200, 150};
    ctx->style = &s;
    if (mu_begin_window_ex(ctx, "menu",
                           mu_rect(c->window.w / 2 - 300, 300, 600, 400),
                           MU_OPT_NOCLOSE | MU_OPT_NORESIZE | MU_OPT_NOFRAME |
                               MU_OPT_NOTITLE)) {
      mu_layout_row(ctx, 1, (int[]){-1}, 0);
      if (mu_button(ctx, "Hrať")) {
        Connector<state_t>::emit(this, "selected", S_LEVEL1);
      }
      if (mu_button(ctx, "Koniec") ||
          controller->key_pressed[Controller::KEY_x]) {
        Connector<state_t>::emit(this, "selected", S_END);
      }
      mu_end_window(ctx);
    }
    ctx->style = olds;
  }
};

struct State {
// where the frames from controller go
#define direct_frames_to(x)                                 \
  frame_conn = Connector<int>::connect(controller, "frame", \
                                       [this](int) { x->render(); })

  unique_ptr<MainMenu> menu;
  unique_ptr<SampleLevel> level1;
  int frame_conn;  // connection from controller frame

  state_t state;
  SDL_Texture *splash_image;

  void showSplash() {
    SDL_SetRenderDrawColor(controller->renderer, 0, 0, 0, 255);
    SDL_RenderClear(controller->renderer);
    SDL_RenderCopy(controller->renderer, splash_image, NULL, NULL);
    SDL_RenderPresent(controller->renderer);
  }

  State() {
    {
      SDL_RWops *rw;
      SDL_Surface *srf;
      string data = Splash::image();
      assert(rw = SDL_RWFromConstMem((const void *)data.data(), data.size()));
      assert(srf = IMG_Load_RW(rw, 1));
      splash_image = SDL_CreateTextureFromSurface(controller->renderer, srf);
      SDL_FreeSurface(srf);
    }

    menu = make_unique<MainMenu>(controller);
    level1 = make_unique<SampleLevel>(controller);
    state = S_MENU;
    showSplash();

    level1->prepare();

    Connector<int>::connect("LevelMap::makeChunkBegin",[this](int){
        showSplash();
        });

    // when level 1 ends, go to menu
    Connector<int>::connect(level1.get(), "finished",
                            [this](int) { setState(S_MENU); });

    // handle menu selections
    Connector<state_t>::connect(menu.get(), "selected",
                                [this](state_t item) { setState(item); });

    // start with events from controller directed to menu
    direct_frames_to(menu);
  }

  void setState(state_t newstate) {
    Connector<int>::disconnect(controller, "frame", frame_conn);

    switch (newstate) {
      case S_MENU:
        SDL_ShowCursor(SDL_ENABLE);
        controller->key_pressed[Controller::KEY_x] = false;
        direct_frames_to(menu);
        break;
      case S_LEVEL1:
        SDL_ShowCursor(SDL_DISABLE);
        direct_frames_to(level1);
        break;
      case S_END:
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#else
        exit(0);
#endif
    }
  }
};

static int mu_text_width(mu_Font font, const char *text, int len) {
  string ff = controller->defaultFont;
  string tt(text);
  if (len < tt.size()) tt.resize(len);
  if (font) ff = string((const char *)(font));
  return controller->textWidth(ff, tt.data());
}
static int mu_text_height(mu_Font font) {
  string ff = controller->defaultFont;
  if (font) ff = string((const char *)(font));
  return controller->textHeight(ff);
}

int main(int argc, char **argv) {
  srandom(time(NULL));
  controller = new Controller("hra", 1200, 800);
  controller->mu_ctx()->text_width = mu_text_width;
  controller->mu_ctx()->text_height = mu_text_height;
  State *s = new State();
  // cout << "state initialized\n";
#ifdef __EMSCRIPTEN__
  emscripten_request_animation_frame_loop(iteration, 0);
  emscripten_force_exit(0);
#else
  controller->run();
#endif
  return 0;
}
