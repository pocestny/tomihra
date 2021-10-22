#ifndef __SAMPLE_LEVEL_H__
#define __SAMPLE_LEVEL_H__
#include "controller.h"
#include "levelmap.h"
#include "sprite.h"
#include <string>
#include <vector>
#include <set>

struct Player {
  uint32_t id=1;
  Sprite *p;
  bool facing_right, walking;
  std::function<void(std::string)> start_new;
  Player(Controller *ctrl);
  uint32_t processInput(Controller *ctrl, Camera *camera, LevelMap *m);
};

struct Monster {
  uint32_t id;
  Sprite *b;
  int tx, ty;
  Monster(Controller *ctrl, LevelMap *m, std::string _name, uint32_t _id);
  void move(Controller *ctrl, LevelMap *m);
  std::string name;
};

struct Brandy {
  uint32_t id=2;
  Sprite *b;
  int tx, ty;
  Brandy(Controller *ctrl, LevelMap *m);
  void move(Controller *ctrl, LevelMap *m);
};

struct SampleLevel {
  bool barrier,brandyfirst,seenbarrier;
  Controller *ctrl;
  LevelMap *m;
  Camera *camera;
  Sprite *strom;
  Player *p;
  std::vector<Monster *> monsters;
  std::set<Monster *> known;
  Brandy *b;

  SampleLevel(Controller *_ctrl);
  void render();
  void prepare();
  double dist2(Sprite *a, Sprite *b);
  void msg(mu_Context *ctx, SDL_Rect *r, std::string title, std::string message);
};

#endif
