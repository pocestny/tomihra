#ifndef __CAMERA_H__
#define __CAMERA_H__

/*
 * a simple helper class that maintains a SDL_Rect of given dimensions
 * center_at() moves camera so that the center is at (x,y), but
 * no part is outside "screen" (it only moves as-far-as-possible)
 *
 * camera has the dimensions of the renderer
 */

#include <SDL2/SDL_rect.h>

#include <ostream>

struct Camera {
  SDL_Rect rect,screen;

  Camera(SDL_Rect *_rect, const SDL_Rect &_screen) : rect{*_rect},screen{_screen} {}
  const SDL_Rect* operator()() const { return &rect; }
  void center_at(int x, int y);
  void move_by(int dx,int dy);
  void normalize();

};

std::ostream& operator<<(std::ostream&, const Camera&);
#endif
