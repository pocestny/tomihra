#ifndef __SAMPLE_LEVEL_H__
#define __SAMPLE_LEVEL_H__
#include "controller.h"
#include "levelmap.h"
#include "sprite.h"
#include "resources.h"
#include <iostream>

using namespace std;
struct SampleLevel {
  Controller *ctrl;
  LevelMap *m;
  Camera *camera;
  Sprite *p;

  SampleLevel(Controller *_ctrl) : ctrl{_ctrl} {
    m = new LevelMap(ctrl->renderer, 16, 256, 256);
    camera = new Camera(ctrl->window);
    p = new Sprite(ctrl->renderer, 48, 48);

    m->addTiles({{0xff, "t1", 0, 16, false},
                 {0xff00ff, "t1", 16, 0, true},
                 {0xffff00ff, "t1", 16, 16, true}});
    m->addTileSheet("t1", SampleLevelResources::tilesheet());
    m->addTileMap(0, SampleLevelResources::mapa());

    p->addCharsheet("c1", SampleLevelResources::charsheet());
    p->addAnimation("none", {{"c1", 0, 0, 700}, {"c1", 0, 48, 1500}});
    Connector<string>::connect(p, "animation_ended",
                            [p=this->p](string) { p->startAnimation("none"); });
    p->startAnimation("none");
  }

  void render_frame() {
    int dx = 0, dy = 0;
    int r = 5;
    if (ctrl->key_pressed[Controller::KEY_UP]) dy = -r;
    if (ctrl->key_pressed[Controller::KEY_DOWN]) dy = r;
    if (ctrl->key_pressed[Controller::KEY_LEFT]) dx = -r;
    if (ctrl->key_pressed[Controller::KEY_RIGHT]) dx = r;
    SDL_Rect npos = *(p->rect());
    npos.x += dx;
    npos.y += dy;
    if (m->accessible(&npos)) p->moveTo(npos.x, npos.y);
    camera->center_at(p->cx(), p->cy(), m->screen());
    m->render(0, (*camera)());
    p->render((*camera)());
  }
};

#endif
