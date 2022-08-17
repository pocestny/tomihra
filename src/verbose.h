#ifndef __VERBOSE_H__
#define __VERBOSE_H__
#include <iostream>
#ifdef VERBOSE
#define dbg std::cerr
#else
static std::ostream dbg(0);
#endif
#endif
