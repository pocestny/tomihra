#include "inventory.h"

#include "controller.h"
#include "pixel.h"
#include "verbose.h"

using namespace std;

/* slots have blue=0, r,g encode x,y */
Inventory::Inventory(SDL_Texture *f, SDL_Surface *m)
    : frame{f}, mask{m}, active{false}, dragging{false} {
  uint32_t format;
  int access;
  SDL_QueryTexture(frame, &format, &access, &camera.w, &camera.h);
  for (int i = 0; i < camera.w; i++)
    for (int j = 0; j < camera.h; j++) {
      uint32_t clr = pixelAt(mask, i, j);
      if (tops.count(clr) == 0) tops[clr] = make_pair(i, j);
    }

  slots.resize(48);
  for (int x = 0; x < 12; x++)
    for (int y = 0; y < 4; y++) {
      int s = slotByGrid(x, y);
      slots[s].empty = true;
      slots[s].x = x;
      slots[s].y = y;
      slots[s].clr = x << 28 | y << 20 | 0xff;
    }

  specialSlots.resize(1);
  specialSlots[0].empty = true;
  specialSlots[0].clr = 0x0000ffff;
}

int Inventory::slotByColor(uint32_t clr) {
  uint32_t r = (clr >> 24) & 0xff, g = (clr >> 16) & 0xff,
           b = (clr >> 8) & 0xff, a = clr & 0xff;
  if (b != 0) return -1;
  return slotByGrid(r >> 4, g >> 4);
}

int Inventory::specialSlotByColor(uint32_t clr) {
  if (clr == 0x0000ffff) return 0;
  return -1;
}

int Inventory::slotByGlobalPos(int mx, int my) {
  mx -= camera.x;
  my -= camera.y;
  if (mx > 0 && my > 0 && mx < camera.w && my < camera.h)
    return slotByColor(pixelAt(mask, mx, my));
  else
    return -1;
}

int Inventory::specialSlotByGlobalPos(int mx, int my) {
  mx -= camera.x;
  my -= camera.y;
  if (mx > 0 && my > 0 && mx < camera.w && my < camera.h)
    return specialSlotByColor(pixelAt(mask, mx, my));
  else
    return -1;
}

int Inventory::firstEmptySlot() {
  for (int i = 0; i < 48; i++)
    if (slots[i].empty) return i;
  return -1;
}

void Inventory::store(uint32_t item, int pos) {
  if (pos == -1) pos = firstEmptySlot();
  slots[pos].empty = false;
  slots[pos].itemID = item;
}

bool Inventory::hasItem(uint32_t id) {
  for (auto &s : slots)
    if (!s.empty && s.itemID == id) return true;
  return false;
}

void Inventory::removeItem(uint32_t id) {
  for (auto &s : slots)
    if (!s.empty && s.itemID == id) {
      s.empty=true;
      return;
    }
}

void Inventory::show(Items &items) {
  SDL_ShowCursor(SDL_ENABLE);
  SDL_RenderCopy(controller->renderer, frame, NULL, &camera);
  drawPortrait();

  if (controller->isLeftClick()) {
    int s = slotByGlobalPos(controller->mouseX, controller->mouseY);
    if (s > -1 && !slots[s].empty) {
      slots[s].empty = true;
      dragging = true;
      draggedItemID = slots[s].itemID;
    } else {
      s = specialSlotByGlobalPos(controller->mouseX, controller->mouseY);
      int w = SpecialSlotItem::Weapon;
      if (s == w && !specialSlots[w].empty) {
        specialSlots[w].empty = true;
        dragging = true;
        draggedItemID = specialSlots[w].itemID;
      }
    }
  } else if (dragging && controller->isLeftRelease()) {
    int s = slotByGlobalPos(controller->mouseX, controller->mouseY);
    dragging = false;
    if (s > -1) {
      if (slots[s].empty)
        store(draggedItemID, s);
      else
        store(draggedItemID);
    } else {
      s = specialSlotByGlobalPos(controller->mouseX, controller->mouseY);
      int w = SpecialSlotItem::Weapon;
      if (specialSlots[w].empty && draggedItemID == 2) {
        specialSlots[w].empty = false;
        specialSlots[w].itemID = draggedItemID;
      } else
        store(draggedItemID);
    }
  }

  for (auto &s : slots)
    if (s.empty == false) items.renderItem(s.itemID, &camera, clrOffset(s.clr));
  for (auto &s : specialSlots)
    if (s.empty == false) items.renderItem(s.itemID, &camera, clrOffset(s.clr));

  if (dragging)
    items.renderItem(
        draggedItemID, &camera,
        make_pair(controller->mouseX - 16, controller->mouseY - 16));

  if (KEY(ESCAPE)) {
    active = false;
    if (dragging) {
      dragging = false;
      store(draggedItemID);
    }
  }
}

pair<int, int> Inventory::clrOffset(uint32_t clr) {
  auto res = tops.at(clr);
  res.first += camera.x;
  res.second += camera.y;
  return res;
}
