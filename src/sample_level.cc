#include "sample_level.h"
#include "resources.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;

#define BARRIER 0xfffff

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

uint32_t Player::processInput(Controller *ctrl, Camera *camera, LevelMap *m) {
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
  uint32_t res = 0;
  SDL_Rect npos = *(p->rect());
  SDL_Rect cpos = *(p->collision_rect());
  npos.x += dx;
  npos.y += dy;
  cpos.x += dx;
  cpos.y += dy;
  if (m->accessible(&cpos) && (!m->isCollision(0, id, &cpos)))
    p->moveTo(npos.x, npos.y);
  else {
    auto c = m->getCollisions(0, &cpos);
    if (find(c.begin(), c.end(), BARRIER) != c.end()) res = BARRIER;
  }
  camera->center_at(p->cx(), p->cy(), m->screen());
  return res;
}

Monster::Monster(Controller *ctrl, LevelMap *m, string _name, uint32_t _id)
    : id{_id}, name{_name} {
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
  SDL_Rect r = *(b->rect());
  double a =
      sqrt(18.0f / (double)((tx - r.x) * (tx - r.x) + (ty - r.y) * (ty - r.y)));

  r.x += a * (tx - r.x);
  r.y += a * (ty - r.y);

  if (m->accessible(&r) && (!m->isCollision(0, id, &r)) &&
      (abs(r.x - tx) > 3) && (abs(r.y - ty) > 3)) {
    b->moveTo(r.x, r.y);
  } else {
    tx = random() % m->px_width();
    ty = random() % m->px_height();
  }
}

Brandy::Brandy(Controller *ctrl, LevelMap *m) {
  b = new Sprite(ctrl->renderer, 50, 40);
  b->addCharsheet("c1", SampleLevelResources::brandy());
  b->addAnimation("idle", {{"c1", 0, 0, 0xffffffff}});
  do b->moveTo(random() % m->px_width(), random() % m->px_height());
  while (!m->accessible(b->rect()));
  b->startAnimation("idle");
  tx = random() % m->px_width();
  ty = random() % m->px_height();
}

void Brandy::move(Controller *ctrl, LevelMap *m) {
  SDL_Rect r = *(b->rect());
  double a =
      sqrt(1.7f / (double)((tx - r.x) * (tx - r.x) + (ty - r.y) * (ty - r.y)));

  r.x += a * (tx - r.x);
  r.y += a * (ty - r.y);

  if (m->accessible(&r) && (!m->isCollision(0, id, &r)) &&
      (abs(r.x - tx) > 30) && (abs(r.y - ty) > 30)) {
    b->moveTo(r.x, r.y);
  } else {
    tx = random() % m->px_width();
    ty = random() % m->px_height();
  }
}

SampleLevel::SampleLevel(Controller *_ctrl)
    : ctrl{_ctrl},
      barrier{true},
      brandyfirst{true},
      seenbarrier{false},
      splash{true} {
  camera = new Camera(ctrl->window);

  // map
  m = new LevelMap(ctrl->renderer, 16, 512, 512, 3000);
  m->addTiles(
      "t1", 16, 16,
      {0,   1,  2,  130, 132, 4,   6,   135, 8,   134, 136, 9,   131, 10,
       11,  16, 17, 18,  147, 146, 21,  148, 150, 152, 22,  32,  33,  34,
       162, 36, 37, 38,  163, 40,  41,  164, 42,  43,  176, 49,  50,  51,
       52,  55, 56, 192, 193, 194, 195, 65,  197, 66,  68,  67,  208, 81,
       210, 83, 82, 84,  87,  88,  224, 225, 226, 227, 229, 114, 115, 116},
      {21, 83, 88, 42, 43});
  m->addTileSheet("t1", SampleLevelResources::tilesheet());
  m->addTileMap(0, SampleLevelResources::mapa());
  m->clearCollisionLayer(0);

  p = new Player(ctrl);
  p->p->moveTo(180 * 16, 370 * 16);

  uint32_t id = 3;
  for (string x : {"Uňu", "Kuňu", "Ňuňu", "Huňu", "Žuňu", "Kleofáš"})
    monsters.push_back(new Monster(ctrl, m, x, id++));

  b = new Brandy(ctrl, m);

  strom = new Sprite(ctrl->renderer, 128, 128);
  strom->addCharsheet("c1", SampleLevelResources::strom());
  strom->addAnimation("idle", {{"c1", 0, 0, 0xffffffff}});
  strom->moveTo(110 * 16, 94 * 16);
  strom->collision_rect()->y = 88;
  strom->collision_rect()->h = 40;
  strom->startAnimation("idle");
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

void SampleLevel::render() {
  if (ctrl->key_pressed[Controller::KEY_x]) {
    Connector<int>::emit(this, "finished", 0);
    return;
  }

  if (splash) {
    SDL_ShowCursor(SDL_ENABLE);
    camera->center_at(p->p->cx(), p->p->cy(), m->screen());
    m->render(0, (*camera)());
    b->b->render((*camera)());
    for (auto b : monsters) b->b->render((*camera)());
    strom->render((*camera)());
    p->p->render((*camera)());
    mu_Style s, *olds;
    auto ctx = ctrl->mu_ctx();
    olds = ctx->style;
    s = *olds;
    s.padding = 10;
    s.colors[MU_COLOR_WINDOWBG] = {255, 255, 255, 150};
    s.colors[MU_COLOR_TITLEBG] = {255, 255, 255, 200};
    s.colors[MU_COLOR_TEXT] = {0, 0, 0, 255};
    s.colors[MU_COLOR_TITLETEXT] = {0, 0, 0, 255};
    s.colors[MU_COLOR_BUTTON] = {50, 90, 250, 200};
    s.colors[MU_COLOR_BUTTONHOVER] = {50, 200, 255, 255};
    s.colors[MU_COLOR_BUTTONFOCUS] = {50, 200, 255, 255};

    ctx->style = &s;
    if (mu_begin_window_ex(ctx, "testovací level",
                           mu_rect(ctrl->window.w / 2 - 300, 50, 600, 200),
                           MU_OPT_NOSCROLL | MU_OPT_NOCLOSE | MU_OPT_NORESIZE |
                               MU_OPT_FORCEGEOM)) {
      mu_layout_row(ctx, 1, (int[]){-1}, 0);
      mu_text(ctx, "Spoznaj všetkých ňuňúrikov a nájdi tajomný Strom.");

      mu_layout_set_next(ctx, mu_rect(270, 90, 60, 40), 1);
      if (mu_button(ctx, "OK")) {
        SDL_ShowCursor(SDL_DISABLE);
        splash = false;
      }
      mu_end_window(ctx);
    }
    ctx->style = olds;
    return;
  }

  uint32_t pres = p->processInput(ctrl, camera, m);
  if ((dist2(p->p, b->b) > 30000)) b->move(ctrl, m);
  for (auto b : monsters) b->move(ctrl, m);

  m->render(0, (*camera)());
  b->b->render((*camera)());
  for (auto b : monsters) {
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
  strom->render((*camera)());
  p->p->render((*camera)());

  if (pres == BARRIER) {
    msg(ctrl->mu_ctx(), p->p->rect(), "",
        "Zdá sa, že tu je bariéra a nedá sa prejsť");
    seenbarrier = true;
  }

  if (dist2(p->p, b->b) < 30000) {
    if (seenbarrier && (barrier || brandyfirst)) {
      msg(ctrl->mu_ctx(), b->b->rect(), "",
          "Ja som Brandy a zrušil som ti bariéru na ostrov.");
      barrier = false;
    } else
      msg(ctrl->mu_ctx(), b->b->rect(), "", "Ahoj, už si bol na ostrove na ostrove?");
  } else if (!barrier)
    brandyfirst = false;

  auto tmp = ctrl->prepareText("Q for run, X for exit");
  ctrl->renderText(tmp, 10, (ctrl->window.h - tmp->h) - 10);

  {
    stringstream ss;
    ss << "poznám: ";
    for (auto b : known) ss << b->name << "    ";
    if (known.size() == monsters.size()) ss << " ... to sú už všetci!";
    ctrl->drawText(ss.str().data(), 10, 10, {0, 0, 0});
  }

  m->clearCollisionLayer(0);
  if (barrier) {
    SDL_Rect r = {.x = 148 * 16, .y = 64 * 16, .w = 10 * 16, .h = 16};
    m->fillCollisionLayer(0, BARRIER, &r);
  }
  m->fillCollisionLayer(0, p->id, p->p->rect());
  m->fillCollisionLayer(0, b->id, b->b->rect());
  m->fillCollisionLayer(0, 0xfffe, strom->rect());
  for (auto b : monsters) m->fillCollisionLayer(0, b->id, b->b->rect());

  if (dist2(strom, p -> p) < 30000) {
    if (known.size() == monsters.size()) {
          SDL_ShowCursor(SDL_ENABLE);
      mu_Style s, *olds;
      auto ctx = ctrl->mu_ctx();
      olds = ctx->style;
      s = *olds;
      s.padding = 10;
      s.colors[MU_COLOR_WINDOWBG] = {255, 255, 255, 150};
      s.colors[MU_COLOR_TITLEBG] = {255, 255, 255, 200};
      s.colors[MU_COLOR_TEXT] = {0, 0, 0, 255};
      s.colors[MU_COLOR_TITLETEXT] = {0, 0, 0, 255};
      ctx->style = &s;
      if (mu_begin_window_ex(ctx, " ",
                             mu_rect(ctrl->window.w / 2 - 300, 50, 600, 200),
                             MU_OPT_NOTITLE| MU_OPT_NOSCROLL | MU_OPT_NOCLOSE |
                                 MU_OPT_NORESIZE | MU_OPT_FORCEGEOM)) {
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_text(ctx, "Gratulujem, vyhral si :-) ");

        mu_layout_set_next(ctx, mu_rect(280, 90, 60, 40), 1);
        if (mu_button(ctx, "OK")) {
          SDL_ShowCursor(SDL_DISABLE);
          Connector<int>::emit(this, "finished", 0);
        }
        mu_end_window(ctx);
      }
      ctx->style = olds;
      return;
    } else {
      msg(ctrl->mu_ctx(), p->p->rect(), "",
          "Ešte treba spoznať všetkých ňuňúrikov.");
    }
  }
}

void SampleLevel::prepare() {
  SDL_Rect r = *(p->p->rect());
  r.x -= m->chunkSize();
  r.y -= m->chunkSize();
  r.w += 2 * m->chunkSize();
  r.h += 2 * m->chunkSize();
  m->makeChunksAt(&r);
}
