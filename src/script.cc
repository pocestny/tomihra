#include "script.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <algorithm>
#include <sstream>

#include "controller.h"
#include "pixel.h"
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
      box(3, controller->renderer, 51, 71),
      m(controller->renderer),
      items(controller->renderer),
      inventory(controller->loadImage(Splash::inv()),
                controller->loadImageSurface(Splash::invMask())),
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
  keyFound = false;
  gameState = Running;

  items.iconsheet = controller->loadImage(Splash::items());
  Item key{.id = 1, .ltype = Item::Hidden};
  key.loc.mloc = {.px = 15 * 24, .py = 79 * 24};
  key.iconRect = {.x = 0, .y = 0, .w = 32, .h = 32};
  items[1] = key;

  Item sword{.id = 2, .ltype = Item::Hidden};
  sword.iconRect = {.x = 32, .y = 0, .w = 32, .h = 32};
  items[2] = sword;

  inventory.camera.x = 100;
  inventory.camera.y = 100;
  inventory.drawPortrait = [this]() {
    SDL_Rect src = playerSelectImage(playerName, playerHasWeapon);
    SDL_Rect dst{.w = 256, .h = 256};
    auto ofs = inventory.clrOffset(0x00ffffff);
    dst.x = ofs.first;
    dst.y = ofs.second;
    SDL_RenderCopy(controller->renderer, playerselect_image, &src, &dst);
  };

  box.collision_rect = {.x = 1, .y = 20, .w = 49, .h = 50};
  box.moveTo(15 * 24, 79 * 24);
  box.addCharsheet("box", DemoLevelResources::box());
  box.addAnimation("idle", {{"box", 0, 0, 0xffffff}});
  Connector<string>::connect(&box, "animation_ended",
                             [this](string) { box.startAnimation("idle"); });
  box.startAnimation("idle");
}

SDL_Rect Script::playerSelectImage(const string &name, bool withWeapon) {
  SDL_Rect src{.x = 0, .y = 0, .w = 256, .h = 256};
  if (playerName == "matka")
    src.x += 256;
  else if (playerName == "katka")
    src.x += 512;
  if (!withWeapon) src.y += 256;
  return src;
}

void Script::showStartScreen() {
  auto r = controller->renderer;
  SDL_ShowCursor(SDL_ENABLE);
  SDL_SetRenderDrawColor(r, 248, 218, 176, 255);
  SDL_RenderFillRect(r, controller->screen());
  SDL_Rect src = playerSelectImage(playerName, true),
           dst{.x = 20, .y = 100, .w = 256, .h = 256};

  SDL_RenderCopy(r, playerselect_image, &src, &dst);

  // menu
  auto ctx = controller->mu_ctx();
  mu_Style s, *olds;
  olds = ctx->style;
  s = *olds;
  s.font = (void *)"large";
  s.padding = 30;
  s.colors[MU_COLOR_BUTTON] = {50, 200, 200, 200};
  s.colors[MU_COLOR_BUTTONHOVER] = {50, 200, 200, 250};
  s.colors[MU_COLOR_BUTTONFOCUS] = {50, 200, 200, 250};
  ctx->style = &s;
  if (mu_begin_window_ex(
          ctx, "menu", mu_rect(400, 100, 600, 400),
          MU_OPT_NOCLOSE | MU_OPT_NORESIZE | MU_OPT_NOFRAME | MU_OPT_NOTITLE)) {
    mu_layout_row(ctx, 1, (int[]){-1}, 0);
    for (auto &p : vector<pair<string, string>>{
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
  return x * x + y * y;
}

float Script::dist(const SDL_Rect &a, int px, int py) {
  float x = a.x - px, y = a.y - py;
  return x * x + y * y;
}

void Script::render() {
  camera.center_at(player.cx(), player.cy());
  m.render(0, camera());
  items.render(camera());

  if (player.rect.y < box.rect.y) {
    player.render(camera());
    box.render(camera());
  } else {
    box.render(camera());
    player.render(camera());
  }

  if (!dragonDead) dragon.render(camera());
  m.render(1, camera());
}

uint32_t Script::trigger(Sprite *player) {
  return m.tiles[m.tile_at(m.layerIDs["proximity"], player->cx(), player->cy())]
      .id;
}

void Script::processGameFrame() {
  /*
  dbg<<"mouse ["<<controller->mouseX<<","<<controller->mouseY<<"]  ";
  for(int i:{0,1,2}) dbg<<" "<<controller->mouseButton[i];
  dbg<<endl;
  */

  float speed = 4;
  if (KEY(LCTRL) || KEY(RCTRL)) speed = 8;

  if (trigger(&player) == 169 && m.layers[m.layerIDs["shine"]].visible) {
    gameState = Won;
    return;
  }
  cl.clear();
  if (!dragonDead) cl.fill(dragon.id, &dragon.rect);
  cl.fill(player.id, &player.collision_rect);
  cl.fill(box.id, &box.collision_rect);

  if (trigger(&player) == 257 && inventory.hasItem(1) && KEY(SPACE)) {
    inventory.removeItem(1);
    items[1].ltype = Item::Hidden;
    m.layers[m.layerIDs["door closed"]].visible = false;
    m.layers[m.layerIDs["door closed"]].collision_active = false;
    m.layers[m.layerIDs["door open"]].visible = true;
  }

  if (!dragonDead && trigger(&player) == 50) {
    // fixme
    if (dist(player.rect, dragon.rect) < 3000 && playerHasWeapon) {
      dragonDead = true;
      m.layers[m.layerIDs["shine"]].visible = true;
    }
    if (dist(player.rect, dragon.rect) < 3000 && !playerHasWeapon) {
      gameState = Lost;
      return;
    }

    if (playerHasWeapon)
      dragon.tryMove(
          dirTo(dragon.cx(), dragon.cy(), 2 * dragon.cx() - player.cx(),
                2 * dragon.cy() - player.cy()),
          1.8);
    else
      dragon.tryMove(dirTo(dragon.cx(), dragon.cy(), player.cx(), player.cy()),
                     1.8);
    if (!m.accessible({0}, &dragon.collision_rect) ||
        cl.isCollision(dragon.id, &dragon.rect) || trigger(&dragon) != 50)
      dragon.revertMove();
  }

  player.tryMove({KEY(UP) || KEY(w), KEY(LEFT) || KEY(a), KEY(DOWN) || KEY(s),
                  KEY(RIGHT) || KEY(d)},
                 speed);

  if (!m.accessible({0}, &player.collision_rect))
    player.revertMove();
  else {
    auto c = cl.getCollisions(&player.collision_rect);
    if (c.size() > 0) {
      bool onlyBox = true;
      bool isbox = false;
      for (uint32_t x : c)
        if (x != 1) {
          SDL_Rect r, tmp;
          if (x == 3)
            r = box.collision_rect;
          else {
            onlyBox = false;
            continue;
          }  // FIXME

          if (SDL_IntersectRect(&r, &player.collision_rect, &tmp) == SDL_TRUE)
            isbox = true;
        }

      if (!onlyBox)
        player.revertMove();
      else if (isbox) {
        SDL_Rect intr;
        SDL_IntersectRect(&player.collision_rect, &box.collision_rect, &intr);
        int dx = player.rect.x - player.old_rect.x;
        int dy = player.rect.y - player.old_rect.y;

        int ofx = 0, ofy = 0;
        if (dx > 0 && player.rect.x < box.rect.x) ofx = intr.w;
        if (dx < 0 && player.rect.x > box.rect.x) ofx = -intr.w;
        if (dy > 0 && player.rect.y < box.rect.y) ofy = intr.h;
        if (dy < 0 && player.rect.y > box.rect.y) ofy = -intr.h;

        /*
        dbg<<"player "<<player.collision_rect<<" box"<<box.collision_rect<<"
        intr"<<intr<< "dx="<<dx<<" dy="<<dy<<" ofx="<<ofx<<" ofy="<<ofy<<endl;
        */

        box.collision_rect.x += ofx;
        box.collision_rect.y += ofy;
        if (m.accessible({0}, &box.collision_rect)) {
          box.rect.x += ofx;
          box.rect.y += ofy;
        } else {
          box.collision_rect.x -= ofx;
          box.collision_rect.y -= ofy;
          player.revertMove();
        }
      }
    }
  }

  if (trigger(&box) == 233) {
    if (!keyFound) items[1].ltype = Item::Map;
    m.layers[m.layerIDs["keyshine"]].visible = true;
  } else
    m.layers[m.layerIDs["keyshine"]].visible = false;

  if (trigger(&player) == 1 && KEY(SPACE) && !chestOpen) {
    chestOpen = true;
    m.layers[m.layerIDs["chest_closed"]].visible = false;
    m.layers[m.layerIDs["chest_closed"]].collision_active = false;
    m.layers[m.layerIDs["chest_open"]].visible = true;
    m.layers[m.layerIDs["chest_open"]].collision_active = true;
    auto &s = items[2];
    s.ltype = Item::Inventory;
    inventory.store(2);
  }

  auto &k = items[1];
  if (k.ltype == Item::Map &&
      dist(player.rect, k.loc.mloc.px, k.loc.mloc.py) < 2000) {
    k.ltype = Item::Inventory;
    keyFound = true;
    inventory.store(1);
  }
}

void Script::endGame() {
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#else
  controller->quit = true;
#endif
}

void Script::onFrame() {
  if (KEY(x) && KEY(LCTRL)) endGame();

  if (startScreen) {
    showStartScreen();
    return;
  }

  {
    auto r = controller->renderer;
    if (gameState == Lost) {
      SDL_SetRenderDrawColor(r, 248, 218, 176, 255);
      SDL_RenderFillRect(r, controller->screen());
      SDL_Rect src = playerSelectImage(playerName, true),
               dst{.x = 20, .y = 100, .w = 256, .h = 256};

      SDL_RenderCopy(r, playerselect_image, &src, &dst);
      controller->drawText("PREHRAL SI", 300, 200);
      return;
    }

    if (gameState == Won) {
      SDL_SetRenderDrawColor(r, 248, 218, 176, 255);
      SDL_RenderFillRect(r, controller->screen());
      SDL_Rect src = playerSelectImage(playerName, true),
               dst{.x = 20, .y = 100, .w = 256, .h = 256};

      SDL_RenderCopy(r, playerselect_image, &src, &dst);
      controller->drawText("VYHRAL SI", 300, 200);
      return;
    }
  }

  if (!inventory.active && KEY(i)) inventory.active = true;

  SDL_ShowCursor(SDL_DISABLE);

  if (!inventory.active) processGameFrame();
  render();
  if (inventory.active) {
    inventory.show(items);
    playerHasWeapon =
        !inventory.specialSlots[Inventory::SpecialSlotItem::Weapon].empty;
    updatePlayerSkin();
  }
}
