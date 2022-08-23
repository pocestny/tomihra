#ifndef __VERBOSE_H__
#define __VERBOSE_H__
/*
 * provide dbg stream
 * based on compile option VERBOSE, dbg is either cerr or null
 * so dbg<<blah prints blah to stderr if complied with -D VERBOSE
 */

#include <iostream>
#include <SDL2/SDL_image.h>

#ifdef VERBOSE
#define dbg std::cerr
#else
static std::ostream dbg(0);
#endif

std::ostream& operator<<(std::ostream& str, const SDL_Rect &r);

#endif
