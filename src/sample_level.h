#ifndef __SAMPLE_LEVEL_H__
#define __SAMPLE_LEVEL_H__
#include "controller.h"
#include "levelmap.h"
#include "sprite.h"
#include <string>
#include <vector>
#include <set>

struct Player {
  Sprite *p;
  bool facing_right, walking;
  std::function<void(std::string)> start_new;
  Player(Controller *ctrl);
  void processInput(Controller *ctrl, Camera *camera, LevelMap *m);
};

struct Monster {
  Sprite *b;
  int tx, ty;
  Monster(Controller *ctrl, LevelMap *m, std::string _name);
  void move(Controller *ctrl, LevelMap *m);
  std::string name;
};

struct SampleLevel {
  Controller *ctrl;
  LevelMap *m;
  Camera *camera;
  Player *p;
  std::vector<Monster *> monsters;
  std::set<Monster *> known;

  SampleLevel(Controller *_ctrl);
  void render_frame();
  double dist2(Sprite *a, Sprite *b);
  void msg(mu_Context *ctx, SDL_Rect *r, std::string title, std::string message);
};

#endif
