#include "controller.h"
#include "sample_level.h"

#include <cassert>
#include <time.h>
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <iostream>

using namespace std;

Controller* controller;

#ifdef __EMSCRIPTEN__
EM_BOOL iteration(double time, void* userData) {
  controller->iteration();
  return EM_TRUE;
}
#endif

int main(int argc, char** argv) {
  srandom(time(NULL));
  controller = new Controller("hra",1200,800);
  SampleLevel* level = new SampleLevel(controller);
  Connector<int>::connect(controller, "frame", [level](int) {level->render_frame();});
#ifdef __EMSCRIPTEN__
  emscripten_request_animation_frame_loop(iteration, 0);
#else
  controller->run();
#endif
}
