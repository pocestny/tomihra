#ifndef __ULPCCHARACTER_H__
#define __ULPCCHARACTER_H__
#include <array>
#include <functional>

#include "sprite.h"

/*
 * character from Universal LPC Spritesheet Generator
 * https://sanderfrenken.github.io/Universal-LPC-Spritesheet-Character-Generator
 *
 * tryMove() changes the animation, and calculates new positions for rect,
 * collision arrowPressed is up,left,down,right new positions can then be
 * checked revertMove() can be used to return to original position
 *
 *
 */

struct ULPCcharacter : public Sprite {
  std::string facing, cycle;
  SDL_Rect old_rect, old_collision_rect;  // used with tryMove
  std::function<void(std::string)> start_new;
  std::string skin;
  ULPCcharacter(
      uint32_t _id, SDL_Renderer* _renderer,
      const std::vector<std::pair<std::string, std::string>> &charsheets);

  void tryMove(const std::array<bool, 4>& arrowsPressed, float speed);
  void revertMove();
};

#endif
