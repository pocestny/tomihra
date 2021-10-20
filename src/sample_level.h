#ifndef __SAMPLE_LEVEL_H__
#define __SAMPLE_LEVEL_H__
#include "controller.h"
#include "levelmap.h"
#include "sprite.h"

using namespace std;
struct SampleLevel {
  Controller *ctrl;
  LevelMap *m;
  Camera *camera;
  Sprite *p;
  bool facing_right,walking;
  function<void(string)> start_new;

  SampleLevel(Controller *_ctrl);
  void render_frame();
};

#endif
