#include "controller.h"

using namespace std;

void Camera::center_at(int x, int y, const SDL_Rect* screen) {
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
void Camera::print() {
  cout << "Camera [" << rect.x << "," << rect.y << "," << rect.w << ","
       << rect.h << "]\n";
}

Controller::Controller(const string& window_name, int width, int height)
    : window{.x = 0, .y = 0, .w = width, .h = height} {
  key_pressed.resize(listened_keys.size(), false);
  key_pressed_old.resize(listened_keys.size(), false);
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
  assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
  assert(sdl_window = SDL_CreateWindow(
             window_name.data(), SDL_WINDOWPOS_UNDEFINED,
             SDL_WINDOWPOS_UNDEFINED, window.w, window.h, SDL_WINDOW_SHOWN));
  assert(renderer =
             SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED));
  assert(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG);
  assert(TTF_Init() != -1);
  loadFont("roboto-regular", Fonts::roboto_regular(), 17);
  loadFont("roboto-bold", Fonts::roboto_bold(), 17);
  defaultFont = "roboto-regular";
  SDL_RenderClear(renderer);
  // render splash screen
  {
    SDL_RWops *rw;
    SDL_Surface *srf;
    string data=Splash::image();
    assert(rw = SDL_RWFromConstMem((const void *)data.data(), data.size()));
    assert(srf=IMG_Load_RW(rw, 1));
    splash_image= SDL_CreateTextureFromSurface(renderer, srf);
    SDL_RenderCopy(renderer,splash_image,NULL,NULL);
    SDL_FreeSurface(srf);
  }
  SDL_RenderPresent(renderer);
}

void Controller::loadFont(std::string name, std::string data, int size) {
  SDL_RWops* tmp;
  font_data.push_back(data);
  assert(tmp = SDL_RWFromConstMem((const void*)font_data.back().data(), data.size()));
  assert(fonts[name] = TTF_OpenFontRW(tmp, 1, size));
}

SDL_Surface* Controller::prepareText(const std::string& text, SDL_Color color,
                                     string font) {
  SDL_Surface* textSurface;
  if (fonts.count(font) == 0) font = defaultFont;
  if (!(textSurface = TTF_RenderText_Solid(fonts[font], text.data(), color))) {
    cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError()
         << endl;
    assert(0);
  }
  return textSurface;
}
void Controller::renderText(SDL_Surface* textSurface, int x, int y) {
  SDL_Texture* texture;
  assert(texture = SDL_CreateTextureFromSurface(renderer, textSurface));
  SDL_Rect renderQuad = {
      .x = x, .y = y, .w = textSurface->w, .h = textSurface->h};
  SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(textSurface);
}

void Controller::renderStat() {
#ifdef  __EMSCRIPTEN__
  auto tmp = prepareText("Q for run");
#else
  auto tmp = prepareText("Q for run, X for exit");
#endif
  renderText(tmp, 10, (window.h - tmp->h) - 10);
}

uint32_t Controller::iteration() {
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
  renderStat();
  SDL_RenderPresent(renderer);
  uint32_t end = SDL_GetTicks();
  return end - start;
}

void Controller::run() {
  bool quit = false;
  while (!quit) {
    uint32_t time = iteration();
    if (key_pressed[KEY_x]) return;
    if (time < ticks_per_frame) SDL_Delay(ticks_per_frame - time);
  }
}

Controller::~Controller() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(sdl_window);
  IMG_Quit();
  SDL_Quit();
}
