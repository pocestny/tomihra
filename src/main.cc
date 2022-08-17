/*
 * lightweight entry point
 */

#include <time.h>

#include <cassert>
#include <cstdlib>
#include <memory>

#include "controller.h"
#include "script.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "verbose.h"

using namespace std;

Controller *controller;

#ifdef __EMSCRIPTEN__
EM_BOOL iteration(double time, void *userData) {
  controller->iteration();
  return EM_TRUE;
}
#endif

// these are for microui - need to be global
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
  auto seed = time(NULL);
  srandom(seed);
  dbg << "rand seed = " << seed << endl;
  controller = new Controller("hra", 1200, 800);
  controller->mu_ctx()->text_width = mu_text_width;
  controller->mu_ctx()->text_height = mu_text_height;
  Script *script = new Script();
  Connector<int>::connect(controller, "frame",
                          [script](int) { script->onFrame(); });
  dbg << "main loop starting\n";
#ifdef __EMSCRIPTEN__
  emscripten_request_animation_frame_loop(iteration, 0);
  emscripten_force_exit(0);
#else
  controller->run();
#endif
  return 0;
}
