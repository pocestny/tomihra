#ifndef ___LEVEL_MAP_H___
#define ___LEVEL_MAP_H___

#include "defines.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

struct Tile {
  uint32_t id;
  std::string tilesheet;
  int x, y;  // absolute position in tilesheet 
  bool passable;
};

class LevelMap {
 protected:
  struct Chunk {
    std::unordered_map<int, SDL_Texture *> layers;
    int px_x, px_y;
    ~Chunk() {
      for (auto x : layers) {
        assert(x.second);
        SDL_DestroyTexture(x.second);
      }
    }
  };

  SDL_Renderer *renderer;  // not owned

  int tile_size, width, height;  // dimensions in tiles
  std::unordered_map<int, SDL_Surface *>
      tilemaps;  // render layer -> map of tiles
  std::unordered_map<std::string, SDL_Surface *> tilesheets;
  std::unordered_map<uint32_t, Tile> tiles;
  std::vector<uint32_t> _layers;

  int chunk_size, chunk_pitch;
  std::vector<Chunk *> chunks;

  inline int chunk_at(int px_x, int px_y) const;
  inline Tile tile_at(int layer, int px_x, int px_y) const;

  void makeChunk(int x, int y);  // in chunk coordinates

 public:
  LevelMap(SDL_Renderer *_renderer, int _tile_size, int _width, int _height,
           int _chunk_size = 4096);
  ~LevelMap();

  const std::vector<uint32_t> layers(){return _layers;}

  void addTiles(const std::vector<Tile> &t);
  void addTileSheet(std::string name, const std::string &data);
  void addTileMap(int layer, const std::string &data);

  void render(int layer, SDL_Rect *camera);
  inline int px_width() const { return tile_size * width; }
  inline int px_height() const { return tile_size * height; }
};

#endif
