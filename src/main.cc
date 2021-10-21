#include "controller.h"
#include "sample_level.h"

#include <cassert>
#include <cstdlib>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <iostream>

using namespace std;

Controller *controller;

#ifdef __EMSCRIPTEN__
EM_BOOL iteration(double time, void *userData) {
  controller->iteration();
  return EM_TRUE;
}
#endif

struct SplashScreen {
  SDL_Texture *splash_image;
  Controller *c;

  SplashScreen(Controller *_c) : c{_c} {
    c->microUi.on = true;
    SDL_RWops *rw;
    SDL_Surface *srf;
    string data = Splash::image();
    assert(rw = SDL_RWFromConstMem((const void *)data.data(), data.size()));
    assert(srf = IMG_Load_RW(rw, 1));
    splash_image = SDL_CreateTextureFromSurface(c->renderer, srf);
    SDL_FreeSurface(srf);
  }
  ~SplashScreen() { SDL_DestroyTexture(splash_image); }
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
        Connector<int>::emit(this, "finished", 0);
      }
      mu_end_window(ctx);
    }
    ctx->style = olds;
  }
};

struct State {
  SplashScreen *splash;
  SampleLevel *level;
  int splash_conn;  // connection to splash screen

  State() {
    splash = new SplashScreen(controller);
    level = new SampleLevel(controller);

    // when splash screen finishes, start level
    Connector<int>::connect(splash, "finished", [this](int) {
      Connector<int>::disconnect(controller, "frame", splash_conn);
      delete (splash);
      splash = nullptr;
      SDL_ShowCursor(SDL_DISABLE);
      Connector<int>::connect(controller, "frame", [level = this->level](int) {
        level->render_frame();
      });
    });

    // events from controller are received by splash screen
    splash_conn = Connector<int>::connect(
        controller, "frame",
        [splash = this->splash](int) { splash->render(); });
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
#else
  controller->run();
#endif
}
