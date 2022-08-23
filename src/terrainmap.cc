#include "terrainmap.h"

#include <cassert>
#include <iostream>

#include "connector.h"
#include "pixel.h"

using namespace std;

TerrainMap::ChunkIterator::ChunkIterator(const SDL_Rect *r, TerrainMap *self) {
  cs = self->chunk_size;
  xlo = (r->x / cs) * cs;
  xhi = r->x + r->w;
  ylo = (r->y / cs) * cs;
  yhi = r->y + r->h;
  cx = xlo;
  cy = ylo;
  x = cx / cs;
  y = cy / cs;
}

TerrainMap::ChunkIterator::operator bool() { return (cy < yhi); }

void TerrainMap::ChunkIterator::operator++() {
  cx += cs;
  if (cx >= xhi) {
    cx = xlo;
    cy += cs;
  }
  x = cx / cs;
  y = cy / cs;
}

TerrainMap::TerrainMap(SDL_Renderer *_renderer, int _tile_size, int _width,
                       int _height, int _chunk_size)
    : renderer{_renderer},
      tile_size{_tile_size},
      width{_width},
      height{_height},
      chunk_size{_chunk_size} {
  screen = {.x = 0, .y = 0, .w = px_width(), .h = px_height()};
  chunk_pitch = px_width() / chunk_size + 1;
  chunks.resize(chunk_pitch * (px_height() / chunk_size + 1), nullptr);
}

TerrainMap::~TerrainMap() {
  for (auto &l : layers) {
    assert(l.surface);
    SDL_FreeSurface(l.surface);
  }
  for (int i = 0; i < chunks.size(); i++)
    if (chunks[i]) delete (chunks[i]);
  for (auto x : tilesheets) {
    assert(x.second->sheet);
    SDL_FreeSurface(x.second->sheet);
    delete x.second;
  }
}

void TerrainMap::addRenderLayer(const string &name, float opacity) {
  layerIDs[name] = layers.size();
  layers.push_back(RenderLayer{nullptr, opacity, true, true});
}

void TerrainMap::addSurfaceToLayer(const string &layer, const string &data) {
  SDL_RWops *tmp;
  SDL_Surface *src, *dst = layers[layerIDs[layer]].surface;
  assert(tmp = SDL_RWFromConstMem((const void *)data.data(), data.size()));
  assert(src = IMG_Load_RW(tmp, 1));
  assert(src->h == height);
  assert(src->w == width);
  assert(src->pitch >= 4 * width);

  if (dst == nullptr)
    layers[layerIDs[layer]].surface = src;
  else
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++) {
        uint32_t val = pixelAt(src, x, y);
        if (val != 0) setPixelAt(dst, x, y, val);
      }
}

void TerrainMap::addStage(const vector<string> &lnames) {
  stages.push_back(vector<int>());
  for (auto &n : lnames) stages.back().push_back(layerIDs[n]);
}

void TerrainMap::addTileSheet(std::string name, const std::string &data,
                              int pitch, int strideX, int strideY) {
  SDL_RWops *tmp;
  SDL_Surface *surface;
  assert(tmp = SDL_RWFromConstMem((const void *)data.data(), data.size()));
  assert(surface = IMG_Load_RW(tmp, 1));
  assert(tilesheets.count(name) == 0);
  tilesheets[name] = new TileSheet{surface, pitch, strideX, strideY};
}

void TerrainMap::addTiles(string sheetname, const vector<uint32_t> &ids,
                          const vector<uint32_t> &t,
                          const unordered_set<uint32_t> solid) {
  TileSheet *sheet = tilesheets.at(sheetname);
  for (int i = 0; i < ids.size(); i++)
    tiles[ids[i]] = Tile{
        ids[i], sheet, sheet->strideX * ((int)t[i] % sheet->pitch),
        sheet->strideY * ((int)t[i] / sheet->pitch), (solid.count(t[i]) == 0)};
}

uint32_t TerrainMap::tile_at(int layer, int px_x, int px_y) const{
  uint32_t res = Tile::Empty;
  int tx = px_x / tile_size, ty = px_y / tile_size;
  return pixelAt(layers[layer].surface, tx, ty);
}

bool TerrainMap::accessible(const vector<int> &inStages, SDL_Rect *rect) {
  if (rect->x + rect->w > screen.w) return false;
  if (rect->y + rect->h > screen.h) return false;
  if (rect->x < 0) return false;
  if (rect->y < 0) return false;
  for (int x = (rect->x / tile_size) * tile_size; x < rect->x + rect->w;
       x += tile_size)
    for (int y = (rect->y / tile_size) * tile_size; y < rect->y + rect->h;
         y += tile_size)
      for (auto s : inStages)
        for (auto l : stages[s])
          if (layers[l].collision_active && !tiles[tile_at(l, x, y)].passable)
            return false;
  return true;
}

int TerrainMap::chunk_at(int px_x, int px_y) const {
  if (px_x < 0) px_x = 0;
  if (px_x > screen.w) px_x = screen.w;
  if (px_y < 0) px_y = 0;
  if (px_y > screen.h) px_y = screen.h;
  return (px_x / chunk_size) + (px_y / chunk_size) * chunk_pitch;
}

bool TerrainMap::processChunkJobs() {
  if (chunkjobs.size() == 0) return false;
  ChunkJob &cj = chunkjobs.back();
  if (cj.srf == nullptr) {
    // start new layer
    assert(cj.srf = SDL_CreateRGBSurface(0, cj.dx, cj.dy, 32, 0xff000000,
                                         0xff0000, 0xff00, 0xff));
    cj.tx = cj.txlo;
    cj.ty = cj.tylo;
  }
  assert(cj.layers.size() > 0);
  for (int numtiles = ChunkJob::MaxTilesPerFrame;
       numtiles > 0 && cj.ty < cj.tyhi; numtiles--) {
    SDL_Rect tile_r = {.x = cj.tx, .y = cj.ty, .w = tile_size, .h = tile_size},
             rect;
    assert(SDL_IntersectRect(&cj.chunk_r, &tile_r, &rect) == SDL_TRUE);
    SDL_Rect src = {.x = rect.x - cj.tx,
                    .y = rect.y - cj.ty,
                    .w = rect.w,
                    .h = rect.h},
             dst = {.x = rect.x - cj.c->px_x,
                    .y = rect.y - cj.c->px_y,
                    .w = rect.w,
                    .h = rect.h};
    Tile &t = tiles[tile_at(cj.layers.back(), cj.tx, cj.ty)];
    src.x += t.x;
    src.y += t.y;

    assert(SDL_BlitSurface(t.tilesheet->sheet, &src, cj.srf, &dst) == 0);
#ifdef DRAW_UNPASSABLE
    if (layers[cj.layers.back()].collision_active && !t.passable) {
      SDL_FillRect(cj.srf, &dst, SDL_MapRGBA(cj.srf->format, 255, 0, 0, 50));
    }
#endif

    cj.tx += tile_size;
    if (cj.tx >= cj.txhi) {
      cj.tx = cj.txlo;
      cj.ty += tile_size;
    }
  }
  if (cj.ty >= cj.tyhi) {
    // surface finished
    int layer = cj.layers.back();
    cj.c->layers[layer] = SDL_CreateTextureFromSurface(renderer, cj.srf);
    SDL_SetTextureAlphaMod(cj.c->layers[layer], 255 * layers[layer].opacity);

    if (cj.c->layers[layer] == nullptr) {
      cout << "CreateTextureFromSurface failed: " << SDL_GetError() << endl;
      assert(0);
    }
    SDL_FreeSurface(cj.srf);
    cj.srf = nullptr;
    cj.layers.pop_back();

    // this was the last layer
    if (cj.layers.size() == 0) {
      chunks[cj.x + cj.y * chunk_pitch] = cj.c;
      chunkjobs.pop_back();
    }
  }
  return chunkjobs.size() > 0;
}

void TerrainMap::makeChunk(int x, int y) {
  if (x < 0) return;
  if (x * chunk_size >= screen.w) return;
  if (y < 0) return;
  if (y * chunk_size >= screen.h) return;
  if (chunks[x + y * chunk_pitch]) return;
  ChunkJob cj;
  cj.c = new Chunk;
  cj.c->px_x = x * chunk_size;
  cj.c->px_y = y * chunk_size;
  for (int i = 0; i < layers.size(); i++) cj.layers.push_back(i);
  cj.x = x;
  cj.y = y;
  cj.srf = nullptr;
  cj.dx = chunk_size;
  cj.dy = chunk_size;
  if (cj.c->px_x + cj.dx > px_width()) cj.dx = px_width() - cj.c->px_x;
  if (cj.c->px_y + cj.dy > px_height()) cj.dy = px_height() - cj.c->px_y;
  cj.chunk_r = {.x = cj.c->px_x, .y = cj.c->px_y, .w = cj.dx, .h = cj.dy};
  cj.txlo = (cj.c->px_x / tile_size) * tile_size;
  cj.txhi = cj.c->px_x + cj.dx;
  cj.tylo = (cj.c->px_y / tile_size) * tile_size;
  cj.tyhi = cj.c->px_y + cj.dy;
  chunkjobs.push_back(cj);
}

void TerrainMap::makeChunksAt(const SDL_Rect *r) {
  for (auto it = ChunkIterator(r, this); it; ++it) makeChunk(it.x, it.y);
}

void TerrainMap::render(int stage, const SDL_Rect *camera) {
  makeChunksAt(camera);
  if (chunkjobs.size() > 0) return;
  for (int layer : stages[stage])
    if (layers[layer].visible)
      for (auto it = ChunkIterator(camera, this); it; ++it) {
        Chunk *c;
        assert(c = chunks[chunk_at(it.cx, it.cy)]);
        assert(c->layers.count(layer) > 0);
        SDL_Rect chunk_r = {.x = it.cx,
                            .y = it.cy,
                            .w = chunk_size,
                            .h = chunk_size},
                 rect;
        assert(SDL_IntersectRect(&chunk_r, camera, &rect) == SDL_TRUE);
        SDL_Rect src = {.x = rect.x - it.cx,
                        .y = rect.y - it.cy,
                        .w = rect.w,
                        .h = rect.h},
                 dst = {.x = rect.x - camera->x,
                        .y = rect.y - camera->y,
                        .w = rect.w,
                        .h = rect.h};
        SDL_RenderCopy(renderer, c->layers[layer], &src, &dst);
      }
}
