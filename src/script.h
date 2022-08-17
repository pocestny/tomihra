#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include<SDL2/SDL_render.h>
#include "camera.h"
#include "demolevel.h"
#include "ulpccharacter.h"
#include "collisionlayer.h"

/*
 * main script that takes care of arranging modules
 * processes "event" and "frame" signals from controller
 */

struct Script {
  SDL_Texture *splash_image,*playerselect_image;
  DemoLevel m;
  ULPCcharacter player,dragon;
  Camera camera;

  std::string playerName;
  bool startScreen;
  bool playerHasWeapon;
  bool chestOpen;
  bool dragonDead;


  CollisionLayer cl;

  Script();
  void onFrame();
  void showStartScreen();
  void processGameFrame();
  void updatePlayerSkin();

  std::array<bool,4> dirTo(int x,int y, int dx, int dy);
  float dist(const SDL_Rect &a, const SDL_Rect &b);
};


#endif
