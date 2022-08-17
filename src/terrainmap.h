#ifndef __TERRAIN_H__
#define __TERRAIN_H__

/*
 * tile-based terrain maps
 *
 * - map consists of tiles. each tile has ID and comes from some tilesheet
 * - all tiles have the same dimension (tile_size x tile_size) px
 * - map has dimensions (width x height) tiles
 * - map has several render layers
 *    render layer has id and tilemap
 *    tilemap is SDL_Surface*  (width x height) where rgba8 pixel stores tileID
 * - rendered map is divided into chunks of (chunk_size x chunk_size) px
 * - chunk owns a rendered SDL_Texture* for each render layer
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct TileSheet {
  SDL_Surface *sheet;
  int pitch,    // number of tiles in a row
      strideX,  // stride in the grid
      strideY;
};
  
struct TileMap{
    SDL_Surface *surface;
    float opacity;
  };


/*
 * descriptor of a tile
 * contains coordinated in given tilesheet, and possibly some properties
 * (passable etc)
 */
struct Tile {
  uint32_t id;
  TileSheet *tilesheet;
  int x, y;  // absolute position in tilesheet
  bool passable;
};

/*
 * main terrain class
 */

struct TerrainMap {
  // chunk starting at pixel position (px_x, px_y) px with all render layers
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

  // iterates over chunks that intersect given rectangle
  struct ChunkIterator {
    int cs,                  // chunk size
        xlo, xhi, ylo, yhi,  // ranges for cx, cy
        cx, cy,              // chunk in pixel coords
        x, y;                // chunk in chunks coords
    ChunkIterator(const SDL_Rect *r, TerrainMap *self);
    operator bool();
    void operator++();
  };

  // one item in chunk job queue
  struct ChunkJob {
    static const int MaxTilesPerFrame = 30000;
    Chunk *c;          // chunk in progress
    SDL_Surface *srf;  // currently generated surface
    int x, y,          // chunks [x + y*chunk_size]
        dx, dy;        // chunk dimensions
    std::vector<uint32_t>
        layers;  // layers to do (currently working on first one)
    SDL_Rect chunk_r;

    int tx, ty, txlo, txhi, tylo, tyhi;  // for iterating over tiles
  };

  /*
   * data
   */

  SDL_Renderer *renderer;  // not owned, stored for convenience from controller

  int tile_size, width, height;  // dimensions in number of tiles

  std::unordered_map<int, TileMap *> tilemaps;  // render layer ->  tilemap
  std::unordered_set<int> visible_layers,
      collision_active_layers;  // switch which layers are active
  std::vector<uint32_t> layers;  // order of render layers (from bottom)
  
  std::unordered_map<std::string, TileSheet *> tilesheets;
  std::unordered_map<uint32_t, Tile> tiles;  // id->tile

  int chunk_size,
      chunk_pitch;                  // number of chunks in a row
  std::vector<Chunk *> chunks;      // stored as [x + y * chunk_pitch]
  std::vector<ChunkJob> chunkjobs;  // chunks that need to be generated

  SDL_Rect screen;  // convenience (0 0 px_width px_height)

  /*
   * methods
   */

  // constuctor / destructor
  TerrainMap(SDL_Renderer *_renderer, int _tilesize, int _width, int _height,
             int _chunk_size = 2048);
  ~TerrainMap();

  inline int px_width() const { return tile_size * width; }
  inline int px_height() const { return tile_size * height; }

  // tiles
  void addTileMap(int layer, const std::string &data,float opacity=1.0);
  void addTileSheet(std::string name, const std::string &data, int pitch,
                    int strideX, int strideY);

  // add tiles with given indices from a tilesheet
  void addTiles(
      std::string sheetname, const std::vector<uint32_t> &ids,
      const std::vector<uint32_t> &t,  // indices of tiles in the sheet
      const std::unordered_set<uint32_t> solid = {});  // which not passable

  // color at given position in tilemap (tile coordinates)
  uint32_t clr_at(int layer, int tx, int ty) const;
  const Tile &tile_at(int layer, int px_x, int px_y) const;

  bool accessible(SDL_Rect *rect);  // are all tiles accessible?

  // chunks
  const int chunkSize() const { return chunk_size; }
  int chunk_at(int px_x, int px_y) const;
  void makeChunksAt(const SDL_Rect *);  // make enought chunks to cover Rect
  void makeChunk(int x, int y);         // in chunk coordinates
  bool processChunkJobs();              //  return true if more work is needed

  void render(int layer, const SDL_Rect *camera);  // render the window
};

#endif
