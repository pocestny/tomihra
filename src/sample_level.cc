#include "sample_level.h"
#include "resources.h"
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

Player::Player(Controller *ctrl) : facing_right{true}, walking{false} {
  p = new Sprite(ctrl->renderer, 48, 48);
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

void Player::processInput(Controller *ctrl, Camera *camera, LevelMap *m) {
  int dx = 0, dy = 0;
  int r = 4;
  bool fr = facing_right, wl = walking;
  // handle input
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

  // move player and camera
  SDL_Rect npos = *(p->rect());
  SDL_Rect cpos = *(p->collision_rect());
  npos.x += dx;
  npos.y += dy;
  cpos.x += dx;
  cpos.y += dy;
  if (m->accessible(&cpos)) p->moveTo(npos.x, npos.y);
  camera->center_at(p->cx(), p->cy(), m->screen());
}

Monster::Monster(Controller *ctrl, LevelMap *m, string _name) : name{_name} {
  b = new Sprite(ctrl->renderer, 40, 40);
  b->addCharsheet("c1", SampleLevelResources::bubo());
  b->addAnimation("idle", {{"c1", 0, 0, 0xffffffff}});
  do b->moveTo(random() % m->px_width(), random() % m->px_height());
  while (!m->accessible(b->rect()));
  b->startAnimation("idle");
  tx = random() % m->px_width();
  ty = random() % m->px_height();
}

void Monster::move(Controller *ctrl, LevelMap *m) {
  while (1) {
    SDL_Rect r = *(b->rect());
    double a = sqrt(
        18.0f / (double)((tx - r.x) * (tx - r.x) + (ty - r.y) * (ty - r.y)));

    r.x += a * (tx - r.x);
    r.y += a * (ty - r.y);

    if (m->accessible(&r) && (abs(r.x - tx) > 3) && (abs(r.y - ty) > 3)) {
      b->moveTo(r.x, r.y);
      break;
    } else {
      tx = random() % m->px_width();
      ty = random() % m->px_height();
    }
  }
}

SampleLevel::SampleLevel(Controller *_ctrl) : ctrl{_ctrl} {
  camera = new Camera(ctrl->window);

  // map
  m = new LevelMap(ctrl->renderer, 16, 256, 256, 4096);
  m->addTiles("t1", 16, 16, {0,  1,  2,  4,  6,  10, 11, 16, 17, 18, 20,
                             21, 22, 32, 33, 34, 36, 37, 38, 42, 43, 49,
                             51, 52, 56, 65, 67, 68, 72, 81, 83, 84, 88},
              {21, 83, 88, 42, 43});
  m->addTileSheet("t1", SampleLevelResources::tilesheet());
  m->addTileMap(0, SampleLevelResources::mapa());

  p = new Player(ctrl);
  for (string x : {"Uňu", "Kuňu", "Ňuňu", "Huňu", "Žuňu", "Kleofáš"})
    monsters.push_back(new Monster(ctrl, m, x));
}

double SampleLevel::dist2(Sprite *a, Sprite *b) {
  int x = a->rect()->x - b->rect()->x, y = a->rect()->y - b->rect()->y;
  return x * x + y * y;
}

void SampleLevel::msg(mu_Context *ctx, SDL_Rect *r, std::string title,
                      std::string message) {
  int padding = 5;
  int w = ctrl->textWidth("", message.data()) + 2 + 2 * padding;
  int h = ctrl->textHeight("") + 2 + 2 * padding;
  mu_Style s, *olds;
  olds = ctx->style;
  s = *olds;
  s.padding = padding;
  s.colors[MU_COLOR_WINDOWBG] = {255, 255, 255, 150};
  s.colors[MU_COLOR_TITLEBG] = {255, 255, 255, 200};
  s.colors[MU_COLOR_TEXT] = {0, 0, 0, 255};
  s.colors[MU_COLOR_TITLETEXT] = {0, 0, 0, 255};
  ctx->style = &s;
  if (mu_begin_window_ex(ctx, title.data(),
                         mu_rect(r->x - (*camera)()->x - w / 2,
                                 r->y - (*camera)()->y - h - 10, w, h),
                         MU_OPT_NOTITLE | MU_OPT_NOSCROLL | MU_OPT_NOCLOSE |
                             MU_OPT_NORESIZE | MU_OPT_FORCEGEOM)) {
    mu_layout_row(ctx, 1, (int[]){-1}, 0);
    mu_text(ctx, message.data());
    mu_end_window(ctx);
  }
  ctx->style = olds;
}

void SampleLevel::render_frame() {
  p->processInput(ctrl, camera, m);
  m->render(0, (*camera)());
  p->p->render((*camera)());
  for (auto b : monsters) {
    b->move(ctrl, m);
    b->b->render((*camera)());
    if (dist2(p->p, b->b) < 30000) {
      stringstream ss;
      ss << "Ahoj, ja som " << b->name;
      known.insert(b);
      // SDL_ShowCursor(SDL_ENABLE);
      msg(ctrl->mu_ctx(), b->b->rect(), b->name.data(), ss.str().data());
    }
    // else SDL_ShowCursor(SDL_DISABLE);
  }
#ifdef __EMSCRIPTEN__
  auto tmp = ctrl->prepareText("Q for run");
#else
  auto tmp = ctrl->prepareText("Q for run, X for exit");
#endif
  ctrl->renderText(tmp, 10, (ctrl->window.h - tmp->h) - 10);

  {
    stringstream ss;
    ss<<"poznám: ";
    for (auto b:known) ss<<b->name<<"    ";
     if (known.size()==monsters.size()) ss<<" ... to sú už všetci!";
    ctrl->drawText(ss.str().data(),10,10, {0,0,0}, "roboto-bold");
  }
}
