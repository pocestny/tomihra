#ifndef __PIXEL_H__
#define __PIXEL_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


uint32_t pixelAt(SDL_Surface *srf, int tx, int ty);
void setPixelAt(SDL_Surface *srf, int tx, int ty, uint32_t val);


#endif
