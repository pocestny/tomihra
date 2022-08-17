#include "controller.h"
#include "verbose.h"

using namespace std;

MicroUI::MicroUI() : on{true} {
  button_map.fill(0);
  button_map[SDL_BUTTON_LEFT & 0xff] = MU_MOUSE_LEFT;
  button_map[SDL_BUTTON_RIGHT & 0xff] = MU_MOUSE_RIGHT;
  button_map[SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE;

  key_map.fill(0);
  key_map[SDLK_LSHIFT & 0xff] = MU_KEY_SHIFT;
  key_map[SDLK_RSHIFT & 0xff] = MU_KEY_SHIFT;
  key_map[SDLK_LCTRL & 0xff] = MU_KEY_CTRL;
  key_map[SDLK_RCTRL & 0xff] = MU_KEY_CTRL;
  key_map[SDLK_LALT & 0xff] = MU_KEY_ALT;
  key_map[SDLK_RALT & 0xff] = MU_KEY_ALT;
  key_map[SDLK_RETURN & 0xff] = MU_KEY_RETURN;
  key_map[SDLK_BACKSPACE & 0xff] = MU_KEY_BACKSPACE;

  mu_ctx = new (mu_Context);
  mu_init(mu_ctx);
}

MicroUI::~MicroUI() { delete mu_ctx; }

void MicroUI::processCommands(Controller* ctrl) {
  mu_Command* cmd = NULL;
  while (mu_next_command(ctrl->mu_ctx(), &cmd)) {
    switch (cmd->type) {
      case MU_COMMAND_TEXT: {
        string font = "";
        if (cmd->text.font) font = string((const char*)(cmd->text.font));
        auto t = ctrl->prepareText(cmd->text.str,
                                   {cmd->text.color.r, cmd->text.color.g,
                                    cmd->text.color.b, cmd->text.color.a},
                                   font);
        ctrl->renderText(t, cmd->text.pos.x, cmd->text.pos.y);
        break;
      }
      case MU_COMMAND_RECT:
        SDL_SetRenderDrawColor(ctrl->renderer, cmd->rect.color.r,
                               cmd->rect.color.g, cmd->rect.color.b,
                               cmd->rect.color.a);
        SDL_RenderFillRect(ctrl->renderer, (SDL_Rect*)&(cmd->rect.rect));
        break;
      case MU_COMMAND_ICON:
        // not yet implemented
        // r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
        break;
      case MU_COMMAND_CLIP:
        SDL_RenderSetClipRect(ctrl->renderer, (SDL_Rect*)&(cmd->clip.rect));
        break;
    }
  }
  SDL_RenderSetClipRect(ctrl->renderer, NULL);
}

Controller::Controller(const string& window_name, int width, int height)
    : bg{0, 0, 0, 0xff}, quit{false} {
  key_pressed.resize(listened_keys.size(), false);
  key_pressed_old.resize(listened_keys.size(), false);
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
  assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"));
  assert(sdl_window = SDL_CreateWindow(
             window_name.data(), SDL_WINDOWPOS_UNDEFINED,
             SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN));
  assert(renderer =
             SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED));
  assert(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG);
  assert(TTF_Init() != -1);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  loadFont("roboto-regular", Fonts::roboto_regular(), 17);
  loadFont("roboto-bold", Fonts::roboto_bold(), 17);
  loadFont("large", Fonts::roboto_bold(), 25);
  defaultFont = "roboto-regular";
}

void Controller::loadFont(std::string name, std::string data, int size) {
  SDL_RWops* tmp;
  font_data.push_back(data);
  assert(tmp = SDL_RWFromConstMem((const void*)font_data.back().data(),
                                  data.size()));
  assert(fonts[name] = TTF_OpenFontRW(tmp, 1, size));
}

SDL_Surface* Controller::prepareText(const char* text, SDL_Color color,
                                     string font) {
  SDL_Surface* textSurface;
  if (fonts.count(font) == 0) font = defaultFont;
  if (!(textSurface = TTF_RenderUTF8_Blended(fonts[font], text, color))) {
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

int Controller::textWidth(std::string font, const char* text) {
  if (fonts.count(font) == 0) font = defaultFont;
  int w, h;
  TTF_SizeText(fonts[font], text, &w, &h);
  return w;
}

int Controller::textHeight(std::string font) {
  if (fonts.count(font) == 0) font = defaultFont;
  int w, h;
  TTF_SizeText(fonts[font], "SampleText", &w, &h);
  return h;
}

SDL_Texture* Controller::loadImage(const std::string& data) {
  SDL_RWops* rw;
  SDL_Surface* srf;
  assert(rw = SDL_RWFromConstMem((const void*)data.data(), data.size()));
  assert(srf = IMG_Load_RW(rw, 1));
  SDL_Texture* img = SDL_CreateTextureFromSurface(renderer, srf);
  SDL_FreeSurface(srf);
  return img;
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
    if (microUi.on) {
      switch (e.type) {
        case SDL_MOUSEMOTION:
          mu_input_mousemove(mu_ctx(), e.motion.x, e.motion.y);
          break;
        case SDL_MOUSEWHEEL:
          mu_input_scroll(mu_ctx(), 0, e.wheel.y * -30);
          break;
        case SDL_TEXTINPUT:
          mu_input_text(mu_ctx(), e.text.text);
          break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
          int b = microUi.button_map[e.button.button & 0xff];
          if (b && e.type == SDL_MOUSEBUTTONDOWN) {
            mu_input_mousedown(mu_ctx(), e.button.x, e.button.y, b);
          }
          if (b && e.type == SDL_MOUSEBUTTONUP) {
            mu_input_mouseup(mu_ctx(), e.button.x, e.button.y, b);
          }
          break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
          int c = microUi.key_map[e.key.keysym.sym & 0xff];
          if (c && e.type == SDL_KEYDOWN) {
            mu_input_keydown(mu_ctx(), c);
          }
          if (c && e.type == SDL_KEYUP) {
            mu_input_keyup(mu_ctx(), c);
          }
          break;
        }
      }
    }
    Connector<SDL_Event>::emit(this, "event", e);
  }
  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
  SDL_RenderClear(renderer);
  if (microUi.on) {
    mu_begin(mu_ctx());
  }
  Connector<int>::emit(this, "frame", 0);
  if (microUi.on) {
    mu_end(mu_ctx());
    microUi.processCommands(this);
  }
  SDL_RenderPresent(renderer);
  uint32_t end = SDL_GetTicks();
  return end - start;
}

void Controller::run() {
  while (!quit) {
    uint32_t time = iteration();
    if (time < ticks_per_frame) SDL_Delay(ticks_per_frame - time);
  }
}

SDL_Rect *Controller::screen() {
  int w, h;
  SDL_GetWindowSize(sdl_window, &w, &h);
  _screen = SDL_Rect{.x = 0, .y = 0, .w = w, .h = h};
  return &_screen;
}

Controller::~Controller() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(sdl_window);
  IMG_Quit();
  SDL_Quit();
}
