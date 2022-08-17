#include "ulpccharacter.h"

#include "verbose.h"
#include <sstream>
#include <tuple>
#include "verbose.h"

#include "connector.h"

using namespace std;
static const string direction[] = {"up", "left", "down", "right"};
static const int scale = 2;

ULPCcharacter::ULPCcharacter(uint32_t _id, SDL_Renderer* _renderer,
                             const vector<pair<string, string>> &charsheets)
    : Sprite(_id, _renderer, 64, 64) {
  facing = "down";
  cycle = "idle";
  skin = charsheets[0].first;
  collision_rect = {.x = 18, .y = 40, .w = 46-18, .h = 64-40};

  for (auto& sheet : charsheets) {
    addCharsheet(sheet.first, sheet.second);
    for (auto& cyc : vector<tuple<string, int, int, int>>{
             {"idle", 8, 1, 15000}, {"walk", 8, 9, 100}}) {
      int row = get<1>(cyc);
      int n = get<2>(cyc);
      int speed = get<3>(cyc);
      for (int i = 0; i < 4; i++) {
        stringstream ss;
        ss <<sheet.first<<"-"<< get<0>(cyc) << "-" << direction[i];
        vector<Frame> anim(n);
        for (int j = 0; j < n; j++) {
          anim[j].charsheet = sheet.first;
          anim[j].x = j * 64;
          anim[j].y = (row + i) * 64;
          anim[j].end_time = (j + 1) * speed;
        }
        addAnimation(ss.str(), anim);
      }
    }
  }
  dbg << "ULPCcharacter "<<_id<<": animations loaded"<<endl;
  for(auto &x:animations) dbg<<"  "<<x.first<<endl;

  start_new = [this](string) {
    stringstream ss;
    ss <<skin<<"-"<< cycle << "-" << facing;
    startAnimation(ss.str());
  };
  Connector<string>::connect(this, "animation_ended", start_new);
  start_new("");
}

void ULPCcharacter::tryMove(const array<bool, 4>& arrowsPressed, float speed) {
  float dx = 0, dy = 0;
  static const array<pair<int, int>, 4> dirs{
      {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}};
  string old_facing = facing, old_cycle = cycle;
  old_rect = rect;
  old_collision_rect = collision_rect;
  cycle = "idle";
  for (int i : {0, 2, 1, 3})
    if (arrowsPressed[i]) {
      cycle = "walk";
      facing = direction[i];
      dx += speed * dirs[i].first;
      dy += speed * dirs[i].second;
    }
  if (old_facing != facing || old_cycle != cycle) start_new("");
  rect.x += dx;
  rect.y += dy;
  collision_rect.x += dx;
  collision_rect.y += dy;
}

void ULPCcharacter::revertMove() {
  rect = old_rect;
  collision_rect = old_collision_rect;
}
