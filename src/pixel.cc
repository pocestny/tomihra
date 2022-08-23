#include "pixel.h"

uint32_t pixelAt(SDL_Surface *srf, int tx, int ty) {
  int offs = ty * srf->pitch + 4 * tx;
  uint32_t res = 0;
  for (int i = 0; i < 4; i++)
    res = (res << 8) | ((uint8_t *)(srf->pixels))[offs + i];
  return res;
}

void setPixelAt(SDL_Surface *srf, int tx, int ty, uint32_t val) {
  int offs = ty * srf->pitch + 4 * tx;
  for (int i = 3; i >= 0; i--, val >>= 8)
    ((uint8_t *)(srf->pixels))[offs + i] = val & 0xff;
}
