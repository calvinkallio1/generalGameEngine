#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "types.h"
#include "sprite.h"
#include "camera.h"

/** A grid of square tiles: a w x h array of 8-bit tile ids, a table saying which ids are solid, and the tileset sprite whose frame index equals the tile id. Tile (tx, ty) sits at world position (tx * tile, ty * tile). Id 0 is empty and never drawn; solid[0] should be false. Fill one from code (point tiles at a static const uint8_t array and solid at a static const bool[256]) or from a Tiled CSV export with tilemap_load_csv(). The struct holds pointers, so it belongs in a scene static, not in GameState; store which level is loaded in GameState and rebuild the map in on_enter.
 *  @field w width in tiles
 *  @field h height in tiles
 *  @field tile size of one tile in pixels (tiles are square)
 *  @field tiles w*h ids, row-major (index = ty * w + tx); 0 = empty
 *  @field solid 256 entries indexed by tile id; true blocks tilemap_move() and tilemap_box_hits()
 *  @field tileset sheet sliced into tile x tile frames; frame index == tile id
 *  @field tints optional: w*h colors, one per cell, same layout as tiles; NULL for none. A cell whose tint has alpha 0 (or is white) is drawn as-is; any other tint multiplies that tile's colors and alpha, as sprite_draw_tinted() does. Draw tintable tiles in neutral greys so one tile becomes any color (a grey house, tinted per lot). Several maps may share one array.
 *  @see tilemap_load_csv, tilemap_draw, tilemap_move, tilemap_at, Sprite, sprite_draw_tinted, gge-tilemap
 */
typedef struct Tilemap {
  int w, h;                 /* in tiles */
  int tile;                 /* tile size in pixels (square) */
  const uint8_t *tiles;     /* w*h ids, row-major; 0 = empty */
  const bool    *solid;     /* indexed by tile id (256 entries) */
  Sprite tileset;           /* frame index == tile id */
  const Color   *tints;     /* optional w*h per-cell tints; NULL = none, alpha 0 = untinted cell */
} Tilemap;

/** The tile id at tile coordinates tx, ty. Coordinates outside the map read as 0 (empty), so callers never need bounds checks. Convert a world position first: tilemap_at(m, (int)floorf(wx / m->tile), (int)floorf(wy / m->tile)).
 *  @return the id, 0 outside the map.
 *  @see tilemap_solid_at, Tilemap
 */
uint8_t tilemap_at(const Tilemap *m, int tx, int ty);             /* 0 outside */
/** Whether the tile under world position wx, wy is solid. Point tests: is the pixel under the mouse a wall, is the ground below a foot solid. Outside the map is not solid.
 *  @return true if solid.
 *  @see tilemap_box_hits, tilemap_at
 */
bool    tilemap_solid_at(const Tilemap *m, float wx, float wy);
/** Whether any solid tile overlaps the world-space rectangle box. Checks every tile the box touches (a box exactly on a tile edge does not count the neighbouring tile). The primitive behind tilemap_move(); use it directly for "can this be placed here" and for non-moving overlap tests.
 *  @return true if box overlaps a solid tile.
 *  @see tilemap_move, tilemap_solid_at
 */
bool    tilemap_box_hits(const Tilemap *m, const Rect *box);
/** Draw every non-empty tile visible through the camera, scaled by the camera's zoom. Culls to camera_visible() plus one tile of margin, so map size does not affect cost. Call from render before drawing entities; call twice with two maps for a background and a foreground layer. If the map has tints, each cell is drawn multiplied by its tint (alpha 0 or white leaves it unmodified); runs of equal tints cost one texture state change, and the tileset's modulation is restored to neutral afterwards.
 *  @see camera_visible, sprite_draw_scaled, sprite_draw_tinted, Tilemap
 */
void    tilemap_draw(const Tilemap *m, const Camera *c);
/** Move an axis-aligned box through the map by vel * dt, stopping at solid tiles. Moves and resolves x first, then y, which is what makes walking along a wall and landing on a floor feel right. On a collision the box is pushed flush against the tile and that velocity component is set to 0; if on_ground is non-NULL it is set to true only when the box was moving down and hit something. Very fast objects (more than a tile per tick) can tunnel; keep speeds below tile / dt or step the call. The platformer and top-down movement core: apply gravity to vel->y, set vel->x from input, call this once per tick.
 *  @see tilemap_box_hits, rect_at, Vec2
 */
void    tilemap_move(const Tilemap *m, Rect *box, Vec2 *vel, float dt, bool *on_ground);

/* Load a Tiled "CSV" layer export (File > Export As > CSV writes one file per layer).
   Allocates m->tiles; call tilemap_free when done. Tiled writes global ids (0 = empty,
   first tile = 1); they are stored as id - 1 so tileset frame N is the tile Tiled shows
   as N. Frame 0 of the sheet is therefore the "empty" tile and is never drawn.
   solid is your own 256-entry table (CSV carries no tile properties).
   Returns false (and logs) on any error. */
/** Load a Tiled "CSV" layer export from assets/ (File > Export As > CSV writes one file per layer; any comma-separated grid of integers with one row per line works). Allocates m->tiles; call tilemap_free() when done (in on_exit). Tiled writes global ids (0 = empty, first tile = 1); they are stored as id - 1 so tileset frame N is the tile Tiled shows as N. Frame 0 of the sheet is therefore the "empty" tile and is never drawn. solid is your own 256-entry table, indexed by the stored id (CSV carries no tile properties); tileset is the sheet texture, sliced into tile_size cells. Ids above 255 wrap; keep tilesets to 256 tiles.
 *  @return true on success; false (and logs) if the file is missing, empty, or a cell is not a number.
 *  @see tilemap_free, assets_texture, Tilemap
 */
bool    tilemap_load_csv(Tilemap *m, const char *relative_path, int tile_size,
                         Texture *tileset, const bool *solid);
/** Release the tile array allocated by tilemap_load_csv() and zero the size. Do not call on a map whose tiles point at a static array.
 *  @see tilemap_load_csv
 */
void    tilemap_free(Tilemap *m);
