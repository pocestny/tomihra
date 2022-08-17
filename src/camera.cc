#include "camera.h"

#include <iostream>
using namespace std;

void Camera::normalize() {
  if (rect.x < screen.x) rect.x = screen.x;
  if (rect.x + rect.w > screen.x + screen.w)
    rect.x = screen.x + screen.w - rect.w;
  if (rect.y < screen.y) rect.y = screen.y;
  if (rect.y + rect.h > screen.y + screen.h)
    rect.y = screen.y + screen.h - rect.h;
}

void Camera::center_at(int x, int y) {
  rect.x = x - rect.w / 2;
  rect.y = y - rect.h / 2;
  normalize();
}

void Camera::move_by(int dx, int dy) {
  rect.x += dx;
  rect.y += dy;
  normalize();
}

ostream& operator<<(ostream& str, const Camera& c) {
  str << "Camera [" << c.rect.x << "," << c.rect.y << "," << c.rect.w << ","
      << c.rect.h << "]";
  return str;
}
