#include "levelmap.h"
#include <cassert>
#include <iostream>
using namespace std;

LevelMap::LevelMap(SDL_Renderer *_renderer, int _tile_size, int _width,
                   int _height, int _chunk_size)
    : renderer{_renderer},
      tile_size{_tile_size},
      width{_width},
      height{_height},
      chunk_size{_chunk_size} {
  _screen = {.x = 0, .y = 0, .w = px_width(), .h = px_height()};
  chunk_pitch = px_width() / chunk_size + 1;
  chunks.resize(chunk_pitch * (px_height() / chunk_size + 1), nullptr);
}

LevelMap::~LevelMap() {
  for (auto x : tilemaps) {
    assert(x.second);
    SDL_FreeSurface(x.second);
  }
  for (auto x : tilesheets) {
    assert(x.second);
    SDL_FreeSurface(x.second);
  }
  for (int i = 0; i < chunks.size(); i++)
    if (chunks[i]) delete (chunks[i]);
}

int LevelMap::chunk_at(int px_x, int px_y) const {
  return (px_x / chunk_size) + (px_y / chunk_size) * chunk_pitch;
}

Tile LevelMap::tile_at(int layer, int px_x, int px_y) const {
  assert(tilemaps.count(layer) > 0);
  int offs =
      (px_y / tile_size) * tilemaps.at(layer)->pitch + 4 * (px_x / tile_size);
  uint32_t id = 0;
  for (int i = 0; i < 4; i++)
    id = (id << 8) | ((uint8_t *)(tilemaps.at(layer)->pixels))[offs + i];
  assert(tiles.count(id) > 0);
  return tiles.at(id);
}

void LevelMap::addTiles(const std::vector<Tile> &t) {
  for (auto x : t) tiles[x.id] = x;
}

void LevelMap::addTileSheet(std::string name, const std::string &data) {
  SDL_RWops *tmp;
  SDL_Surface *tilesheet;
  assert(tmp = SDL_RWFromConstMem((const void *)data.data(), data.size()));
  assert(tilesheet = IMG_Load_RW(tmp, 1));
  if (tilesheets.count(name) > 0) SDL_FreeSurface(tilesheets[name]);
  tilesheets[name] = tilesheet;
}

void LevelMap::addTileMap(int layer, const std::string &data) {
  SDL_RWops *tmp;
  SDL_Surface *tilemap;
  assert(tmp = SDL_RWFromConstMem((const void *)data.data(), data.size()));
  assert(tilemap = IMG_Load_RW(tmp, 1));
  assert(tilemap->h == height);
  assert(tilemap->w == width);
  assert(tilemap->pitch >= 4 * width);

  if (tilemaps.count(layer) > 0)
    SDL_FreeSurface(tilemaps[layer]);
  else
    _layers.push_back(layer);
  tilemaps[layer] = tilemap;
}

void LevelMap::makeChunk(int x, int y) {
  // cout<<"LevelMap::makeChunk("<<x<<","<<y<<")\n";
  if (chunks[x + y * chunk_pitch]) delete (chunks[x + y * chunk_pitch]);
  Chunk *c = new Chunk;
  c->px_x = x * chunk_size;
  c->px_y = y * chunk_size;
  assert(renderer);
  for (auto layer : _layers) {
    int dx = chunk_size, dy = chunk_size;
    if (c->px_x + dx > px_width()) dx = px_width() - c->px_x;
    if (c->px_y + dy > px_height()) dy = px_height() - c->px_y;
    SDL_Surface *tmp;
    assert(tmp = SDL_CreateRGBSurface(0, dx, dy, 32, 0xff000000, 0xff0000,
                                      0xff00, 0xff));
    SDL_Rect chunk_r = {.x = c->px_x, .y = c->px_y, .w = dx, .h = dy};
    for (int tx = (c->px_x / tile_size) * tile_size; tx < c->px_x + dx;
         tx += tile_size)
      for (int ty = (c->px_y / tile_size) * tile_size; ty < c->px_y + dy;
           ty += tile_size) {
        SDL_Rect tile_r = {.x = tx, .y = ty, .w = tile_size, .h = tile_size},
                 rect;
        assert(SDL_IntersectRect(&chunk_r, &tile_r, &rect) == SDL_TRUE);
        SDL_Rect src = {.x = rect.x - tx,
                        .y = rect.y - ty,
                        .w = rect.w,
                        .h = rect.h},
                 dst = {.x = rect.x - c->px_x,
                        .y = rect.y - c->px_y,
                        .w = rect.w,
                        .h = rect.h};
        Tile t = tile_at(layer, tx, ty);
        assert(tilesheets.count(t.tilesheet) > 0);
        src.x += t.x;
        src.y += t.y;
        assert(SDL_BlitSurface(tilesheets[t.tilesheet], &src, tmp, &dst) == 0);
      }
    c->layers[layer] = SDL_CreateTextureFromSurface(renderer, tmp);
    if (c->layers[layer] == nullptr) {
      cout << "CreateTextureFromSurface failed: " << SDL_GetError() << endl;
      assert(0);
    }
    SDL_FreeSurface(tmp);
  }
  chunks[x + y * chunk_pitch] = c;
}

void LevelMap::render(int layer, const SDL_Rect *camera) {
  // cout<<"LevelMap::render("<<layer<<",["<<camera->x<<","<<camera->y<<","<<camera->w<<","<<camera->h<<"]\n";
  for (int cx = (camera->x / chunk_size) * chunk_size;
       cx < camera->x + camera->w; cx += chunk_size)
    for (int cy = (camera->y / chunk_size) * chunk_size;
         cy < camera->y + camera->h; cy += chunk_size) {
      if (!chunks[chunk_at(cx, cy)])
        makeChunk(cx / chunk_size, cy / chunk_size);
      Chunk *c;
      assert(c = chunks[chunk_at(cx, cy)]);
      assert(c->layers.count(layer) > 0);
      SDL_Rect chunk_r = {.x = cx, .y = cy, .w = chunk_size, .h = chunk_size},
               rect;
      assert(SDL_IntersectRect(&chunk_r, camera, &rect) == SDL_TRUE);
      SDL_Rect src = {.x = rect.x - cx,
                      .y = rect.y - cy,
                      .w = rect.w,
                      .h = rect.h},
               dst = {.x = rect.x - camera->x,
                      .y = rect.y - camera->y,
                      .w = rect.w,
                      .h = rect.h};
      SDL_RenderCopy(renderer, c->layers[layer], &src, &dst);
    }
}

bool LevelMap::accessible(SDL_Rect *rect) {
  if (rect->x + rect->w > _screen.w) return false;
  if (rect->y + rect->h > _screen.h) return false;
  if (rect->x < 0) return false;
  if (rect->y < 0) return false;
  for (int x = (rect->x / tile_size) * tile_size; x < rect->x + rect->w;
       x += tile_size)
    for (int y = (rect->y / tile_size) * tile_size; y < rect->y + rect->h;
         y += tile_size)
      for (auto l : _layers)
        if (!tile_at(l, x, y).passable) return false;
  return true;
}
