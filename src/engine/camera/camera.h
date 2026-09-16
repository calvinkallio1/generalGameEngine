#pragma once
#include "types.h"
#include "mathx.h"

typedef struct Camera {
  Vec2  pos;
  float zoom;
  int   view_w, view_h;   /* logical size */
  Vec2  min, max;         /* world bounds; min == max disables */
  Vec2  offset;           /* added at draw time, e.g. screen shake */
} Camera;

void      camera_init(Camera *c, int view_w, int view_h);
Vec2      camera_to_screen(const Camera *c, Vec2 world);
Vec2      camera_to_world(const Camera *c, Vec2 screen);
Rect camera_rect(const Camera *c, Rect world);
Rect camera_visible(const Camera *c);
void      camera_follow(Camera *c, Vec2 target, float smoothing, float dt);   /* smoothing 0 = snap */
void      camera_set_bounds(Camera *c, float x, float y, float w, float h);
