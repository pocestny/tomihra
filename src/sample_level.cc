#include "sample_level.h"
#include "resources.h"
#include <iostream>
#include <sstream>

using namespace std;

SampleLevel::SampleLevel(Controller *_ctrl)
    : ctrl{_ctrl}, facing_right{true}, walking{false} {
  m = new LevelMap(ctrl->renderer, 16, 256, 256, 4096);
  camera = new Camera(ctrl->window);
  p = new Sprite(ctrl->renderer, 48, 48);

  m->addTiles("t1", 16, 16, {0,  1,  2,  4,  6,  10, 11, 16, 17, 18, 20,
                             21, 22, 32, 33, 34, 36, 37, 38, 42, 43, 49,
                             51, 52, 56, 65, 67, 68, 72, 81, 83, 84, 88},
              {21, 83, 88, 42, 43});
  m->addTileSheet("t1", SampleLevelResources::tilesheet());
  m->addTileMap(0, SampleLevelResources::mapa());

  p->addCharsheet("c1", SampleLevelResources::charsheet());
  p->collision_rect()->y = 32;
  p->collision_rect()->h = 16;

  p->addAnimation("idle-right", {{"c1", 0, 48, 400}});
  p->addAnimation("walk-right", {{"c1", 48, 48, 150}, {"c1", 2 * 48, 48, 300}});

  p->addAnimation("idle-left", {{"c1", 3 * 48, 48, 400}});
  p->addAnimation("walk-left",
                  {{"c1", 4 * 48, 48, 150}, {"c1", 5 * 48, 48, 300}});

  p->addAnimation("idle-right-float", {{"c1", 0, 0, 400}});
  p->addAnimation("walk-right-float",
                  {{"c1", 48, 0, 150}, {"c1", 2 * 48, 0, 300}});

  p->addAnimation("idle-left-float", {{"c1", 3 * 48, 0, 400}});
  p->addAnimation("walk-left-float",
                  {{"c1", 4 * 48, 0, 150}, {"c1", 5 * 48, 0, 300}});

  p->moveTo(57 * 16, 37 * 16);
  start_new = [&](string) {
    stringstream ss;
    if (walking)
      ss << "walk";
    else
      ss << "idle";
    if (facing_right)
      ss << "-right";
    else
      ss << "-left";
    if (random() % 100 < 30) ss << "-float";
    p->startAnimation(ss.str());
  };

  Connector<string>::connect(p, "animation_ended", start_new);
  p->startAnimation("idle-right");
}

void SampleLevel::render_frame() {
  int dx = 0, dy = 0;
  int r = 4;
  bool fr = facing_right, wl = walking;

  if (ctrl->key_pressed[Controller::KEY_q]) r = 14;

  walking = false;
  if (ctrl->key_pressed[Controller::KEY_UP]) {
    dy = -r;
    walking = true;
  }
  if (ctrl->key_pressed[Controller::KEY_DOWN]) {
    dy = r;
    walking = true;
  }
  if (ctrl->key_pressed[Controller::KEY_LEFT]) {
    dx = -r;
    facing_right = false;
    walking = true;
  }
  if (ctrl->key_pressed[Controller::KEY_RIGHT]) {
    dx = r;
    facing_right = true;
    walking = true;
  }
  if (fr != facing_right || wl != walking) start_new("");

  SDL_Rect npos = *(p->rect());
  SDL_Rect cpos = *(p->collision_rect());
  npos.x += dx;
  npos.y += dy;
  cpos.x += dx;
  cpos.y += dy;
  //cout<<"npos: ["<<npos.x<<","<<npos.y<<","<<npos.w<<","<<npos.h<<"] ";
  //cout<<"cpos: ["<<cpos.x<<","<<cpos.y<<","<<cpos.w<<","<<cpos.h<<"] \n";

  if (m->accessible(&cpos)) p->moveTo(npos.x, npos.y);
  camera->center_at(p->cx(), p->cy(), m->screen());
  m->render(0, (*camera)());
  p->render((*camera)());
}
