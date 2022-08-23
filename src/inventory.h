#ifndef __INVENTORY_H__
#define __INVENTORY_H__
#include <SDL2/SDL_render.h>

#include <functional>
#include <unordered_map>
#include <vector>

#include "items.h"

struct Inventory {

  //FIXME: unify interface for grid- and special-slots
  struct SlotItem {
    bool empty;
    int x, y;
    uint32_t itemID, clr;
  };

  struct SpecialSlotItem {
    enum { Weapon = 0 } stype;
    uint32_t clr, itemID;
    bool empty;
  };

  SDL_Renderer *renderer;
  SDL_Texture *frame;
  SDL_Surface *mask;
  bool active;
  std::vector<SlotItem> slots;
  std::vector<SpecialSlotItem> specialSlots;
  int slotByGrid(int x, int y) {
    return y * 12 + x;
  }  // index of slot in the grid
  int slotByColor(uint32_t clr);
  int specialSlotByColor(uint32_t clr);
  int slotByGlobalPos(int mx, int my);  // slot from mouse position, or -1
  int specialSlotByGlobalPos(int mx, int my);  
  int firstEmptySlot();
  bool hasItem(uint32_t id);
  void removeItem(uint32_t id);

  std::function<void(void)> drawPortrait;

  std::unordered_map<uint32_t, std::pair<int, int>> tops;
  SDL_Rect camera;

  Inventory(SDL_Texture *f, SDL_Surface *m);
  void show(Items &items);
  std::pair<int, int> clrOffset(uint32_t clr);  // including camera

  void store(uint32_t item,
             int pos = -1);  // store item into slot (-1 = first empty slot)
  bool dragging;
  uint32_t draggedItemID;
};

#endif
