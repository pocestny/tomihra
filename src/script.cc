#include "script.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <sstream>

#include "controller.h"
#include "resources.h"
#include "verbose.h"

using namespace std;

Script::Script()
    : player(1, controller->renderer,
             {
                 {"tom", DemoLevelResources::player_tom()},
                 {"katka", DemoLevelResources::player_katka()},
                 {"matka", DemoLevelResources::player_matka()},
                 {"tom-weapon", DemoLevelResources::player_tom_weapon()},
                 {"katka-weapon", DemoLevelResources::player_katka_weapon()},
                 {"matka-weapon", DemoLevelResources::player_matka_weapon()},
             }),
      dragon(2, controller->renderer,
             {{"dragon", DemoLevelResources::dragon()}}),
      m(controller->renderer),
      camera(controller->screen(),
             SDL_Rect{.x = 0, .y = 0, .w = 24 * 150, .h = 24 * 150}),
      cl(m.tile_size, m.width, m.height) {
  player.moveTo(1840, 1900);
  dragon.moveTo(26 * 24, 52 * 24);

  playerselect_image = controller->loadImage(Splash::playerselect());
  SDL_Rect r{
      .x = 0, .y = 0, .w = m.tile_size * m.width, .h = m.tile_size * m.height};
  m.makeChunksAt(&r);

  playerName = "tom";
  startScreen = true;
  playerHasWeapon = false;
  chestOpen = false;
  dragonDead = false;
}

void Script::showStartScreen() {
  auto r = controller->renderer;
  SDL_ShowCursor(SDL_ENABLE);
  SDL_SetRenderDrawColor(r, 248, 218, 176, 255);
  SDL_RenderFillRect(r, controller->screen());
  SDL_Rect src{.x = 0, .y = 0, .w = 256, .h = 256},
      dst{.x = 20, .y = 100, .w = 256, .h = 256};
  if (playerName == "matka")
    src.x += 256;
  else if (playerName == "katka")
    src.x += 512;
  SDL_RenderCopy(r, playerselect_image, &src, &dst);

  // menu
  auto ctx = controller->mu_ctx();
  mu_Style s, *olds;
  olds = ctx->style;
  s = *olds;
  s.font = (void*)"large";
  s.padding = 30;
  s.colors[MU_COLOR_BUTTON] = {50, 200, 200, 200};
  s.colors[MU_COLOR_BUTTONHOVER] = {50, 200, 200, 250};
  s.colors[MU_COLOR_BUTTONFOCUS] = {50, 200, 200, 250};
  ctx->style = &s;
  if (mu_begin_window_ex(
          ctx, "menu", mu_rect(400, 100, 600, 400),
          MU_OPT_NOCLOSE | MU_OPT_NORESIZE | MU_OPT_NOFRAME | MU_OPT_NOTITLE)) {
    mu_layout_row(ctx, 1, (int[]){-1}, 0);
    for (auto& p : vector<pair<string, string>>{
             {"tom", "Tom"}, {"matka", "NyuNyu"}, {"katka", "Kate"}})
      if (mu_button(ctx, p.second.data())) playerName = p.first;

    if (m.chunkjobs.size() == 0 && mu_button(ctx, "Hrať")) {
      updatePlayerSkin();
      startScreen = false;
    } else
      m.processChunkJobs();
    mu_end_window(ctx);
  }
  ctx->style = olds;
}

void Script::updatePlayerSkin() {
  stringstream ss;
  ss << playerName << ((playerHasWeapon) ? "-weapon" : "");
  player.skin = ss.str();
  player.start_new("");
}

array<bool, 4> Script::dirTo(int x, int y, int dx, int dy) {
  array<bool, 4> res{false, false, false, false};
  static const int n = 10;
  if (dx > x + n) res[3] = true;
  if (dx < x - n) res[1] = true;
  if (dy > y + n) res[2] = true;
  if (dy < y - n) res[0] = true;
  return res;
}

float Script::dist(const SDL_Rect &a, const SDL_Rect &b) {
  float x = a.x - b.x, y = a.y - b.y;
  return x*x+y*y;
}

void Script::processGameFrame() {
  SDL_ShowCursor(SDL_DISABLE);
  float speed = 4;
  if (KEY(LCTRL) || KEY(RCTRL)) speed = 8;

  cl.clear();
  if (!dragonDead) cl.fill(dragon.id, &dragon.rect);
  cl.fill(player.id, &player.rect);

  if (!dragonDead && m.tile_at(7, player.cx(), player.cy()).id == 50) {
    // player in arena

    // fixme
    if (dist(player.rect,dragon.rect)<3000&&playerHasWeapon) {
      dragonDead=true;
      m.visible_layers.insert(9);
    }

    dragon.tryMove(dirTo(dragon.cx(), dragon.cy(), player.cx(), player.cy()),
                   1.8);
    if (!m.accessible(&dragon.collision_rect) ||
        cl.isCollision(dragon.id, &dragon.rect) ||
        m.tile_at(7,dragon.cx(),dragon.cy()).id!=50
        )
      dragon.revertMove();
  }

  player.tryMove({KEY(UP) || KEY(w), KEY(LEFT) || KEY(a), KEY(DOWN) || KEY(s),
                  KEY(RIGHT) || KEY(d)},
                 speed);

  if (!m.accessible(&player.collision_rect) ||
      cl.isCollision(player.id, &player.rect))
    player.revertMove();
  if (m.tile_at(7, player.cx(), player.cy()).id == 1 && KEY(SPACE) &&
      !chestOpen) {
    chestOpen = true;
    playerHasWeapon = true;
    m.visible_layers.erase(4);
    m.collision_active_layers.erase(4);
    m.visible_layers.insert(5);
    m.collision_active_layers.insert(5);
    updatePlayerSkin();
  }
  /*
  if (KEY(SPACE) && !PREV_KEY(SPACE)) {
    player.skin = nextSkin.at(player.skin);
    player.start_new("");
  }
  */

  camera.center_at(player.cx(), player.cy());
  m.render_stage(0, camera());
  player.render(camera());
  if (!dragonDead) dragon.render(camera());
  m.render_stage(1, camera());
}

void Script::onFrame() {
  if (startScreen)
    showStartScreen();
  else
    processGameFrame();

  if (KEY(x)) {
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#else
    controller->quit = true;
#endif
  }
}
