#include "verbose.h"
using namespace std;

ostream& operator<<(ostream& str, const SDL_Rect &r) {
  str<<"Rect{.x="<<r.x<<", .y="<<r.y<<", .w="<<r.w<<", .h="<<r.h<<"}";
  return str;
}
