#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

/*
 * takes care of low-level stuff
 *
 * data:
 *    MicroUI instance
 *        Controller::mu_ctx() gives context
 *        can be excluded from iteration (Controller::microUI::on)
 *
 * constructor:
 *    initialize and open window,
 *    prepare fonts (from resources),
 *    prepare microui instance
 *
 * iteration:
 *   process events. for each event update key status, dispatch to microUI, emit
 * "event" emit "frame" render frame return elapsed time
 *
 * run:
 *  repeat iteration throttled to maxFPS until quit is set
 *
 * helpers:
 *  int           textHeight(string font)
 *  int           textWidth(string font, const char* text)
 *  void          renderText(SDL_Surface* textSurface, int x, int y)
 *  SDL_Surface*  prepareText(const char* text, SDL_Color color, string font)
 *  void          drawText(const char* text, int x, int y, SDL_Color color,
 * string font) SDL_Texture*  loadImage(string data)
 *
 * TODO:
 *  process resize events
 *  render icons in microui
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "connector.h"
#include "microui.h"
#include "resources.h"

struct Controller;
struct MicroUI {
  std::array<char, 256> button_map, key_map;
  mu_Context* mu_ctx;
  bool on;

  MicroUI();
  ~MicroUI();
  void processCommands(Controller* ctrl);
};

struct Controller {
  const int maxFPS = 60;
  const int ticks_per_frame = (1000 / maxFPS);

  // keep track of the pressed state of these keys
  const std::vector<SDL_Keycode> listened_keys{
      SDLK_x,     SDLK_q,      SDLK_UP,     SDLK_DOWN,  SDLK_LEFT,
      SDLK_RIGHT, SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LCTRL, SDLK_RCTRL, SDLK_SPACE, SDLK_a,
      SDLK_s,     SDLK_d,      SDLK_w,};
  enum {
    KEY_x = 0,
    KEY_q,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_SPACE,
    KEY_a,
    KEY_s,
    KEY_d,
    KEY_w
  };
  std::vector<bool> key_pressed,  // is the key pressed now?
      key_pressed_old;            // was it pressed in previous iteration?
  std::string defaultFont;

  SDL_Window* sdl_window;
  SDL_Renderer* renderer;
  SDL_Color bg;
  SDL_Rect _screen; // static fuffer for the screen() method

  MicroUI microUi;

  std::unordered_map<std::string, TTF_Font*> fonts;
  std::vector<std::string> font_data;

  Controller(const std::string& window_name, int width, int height);
  inline mu_Context* const mu_ctx() const { return microUi.mu_ctx; }

  void loadFont(std::string name, std::string data, int size);
  SDL_Surface* prepareText(const char* text, SDL_Color color = {0, 0, 0},
                           std::string font = "");  // allocates the surface
  void renderText(SDL_Surface*, int x, int y);      // frees the sufrace
  inline void drawText(const char* text, int x, int y,
                       SDL_Color color = {0, 0, 0}, std::string font = "") {
    renderText(prepareText(text, color, font), x, y);
  }
  int textWidth(std::string font, const char* text);
  int textHeight(std::string font);
  SDL_Texture* loadImage(const std::string&);
  SDL_Rect *screen();
  uint32_t iteration();
  bool quit;
  void run();
  ~Controller();
};

extern Controller *controller;
#define KEY(x) (controller->key_pressed[Controller::KEY_##x])
#define PREV_KEY(x) (controller->key_pressed_old[Controller::KEY_##x])

#endif
