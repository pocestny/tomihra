#ifndef ___LEVEL_MAP_H___
#define ___LEVEL_MAP_H___

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Tile {
  uint32_t id;  // global id
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

  int tile_size, width, height;  // dimensions in number of tiles
  std::unordered_map<int,std::vector<uint32_t>> collisionLayers;
  std::unordered_map<int, SDL_Surface *>
      tilemaps;  // render layer -> rgba8 surface of tiles (coded as colors)

  std::unordered_map<std::string, SDL_Surface *> tilesheets;
  std::unordered_map<uint32_t, Tile> tiles;  // id->tile
  std::vector<uint32_t> _layers;             // list of render layers

  int chunk_size, chunk_pitch;
  std::vector<Chunk *> chunks;

  int chunk_at(int px_x, int px_y) const;  // chunk index at pixel coordinates

  // color at given position in tilemap
  uint32_t clr_at(int layer, int tx, int ty) const;

  // tile at given pixel coordinates
  Tile virtual tile_at(int layer, int px_x, int px_y) const;

  void makeChunk(int x, int y);  // in chunk coordinates

  SDL_Rect _screen;

 public:
  LevelMap(SDL_Renderer *_renderer, int _tile_size, int _width, int _height,
           int _chunk_size = 2048);
  ~LevelMap();

  const int chunkSize() const {return chunk_size;}
  const SDL_Rect *screen() const { return &_screen; }
  const std::vector<uint32_t> layers() { return _layers; }

  void addTiles(const std::vector<Tile> &t);
  void addTiles(std::string sheet, int pitch, int tilesize,
                const std::vector<uint32_t> &t,
                const std::unordered_set<uint32_t> solid = {});

  void addTileSheet(std::string name, const std::string &data);
  void addTileMap(int layer, const std::string &data);

  void render(int layer, const SDL_Rect *camera);
  inline int px_width() const { return tile_size * width; }
  inline int px_height() const { return tile_size * height; }
  void makeChunksAt(SDL_Rect *);

  bool accessible(SDL_Rect *rect); // are all tiles accessible?
  bool isCollision(int l,uint32_t v, SDL_Rect *rect); // is there a collision without value v?
  std::vector<uint32_t> getCollisions(int l, SDL_Rect *rect); // get all collision values

  void clearCollisionLayer(int l);
  void fillCollisionLayer(int l, uint32_t v, SDL_Rect *rect);

  void printScreen() {
    std::cout << "LevelMap::printScreen=[" << _screen.x << "," << _screen.y
              << "," << _screen.w << "," << _screen.h << "]\n";
  }
};

#endif
