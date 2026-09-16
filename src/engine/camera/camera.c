#include "camera.h"

void camera_init(Camera *c, int view_w, int view_h) {
  *c = (Camera){ .zoom = 1.0f, .view_w = view_w, .view_h = view_h };
}

Vec2 camera_to_screen(const Camera *c, Vec2 w) {
  return (Vec2){ (w.x - c->pos.x - c->offset.x) * c->zoom + c->view_w * 0.5f,
                 (w.y - c->pos.y - c->offset.y) * c->zoom + c->view_h * 0.5f };
}

Vec2 camera_to_world(const Camera *c, Vec2 s) {
  return (Vec2){ (s.x - c->view_w * 0.5f) / c->zoom + c->pos.x + c->offset.x,
                 (s.y - c->view_h * 0.5f) / c->zoom + c->pos.y + c->offset.y };
}

SDL_FRect camera_rect(const Camera *c, SDL_FRect r) {
  Vec2 p = camera_to_screen(c, (Vec2){ r.x, r.y });
  return (SDL_FRect){ p.x, p.y, r.w * c->zoom, r.h * c->zoom };
}

SDL_FRect camera_visible(const Camera *c) {
  float w = c->view_w / c->zoom, h = c->view_h / c->zoom;
  return (SDL_FRect){ c->pos.x + c->offset.x - w * 0.5f, c->pos.y + c->offset.y - h * 0.5f, w, h };
}

void camera_set_bounds(Camera *c, float x, float y, float w, float h) {
  c->min = (Vec2){ x, y }; c->max = (Vec2){ x + w, y + h };
}

void camera_follow(Camera *c, Vec2 target, float smoothing, float dt) {
  c->pos = v2_lerp(c->pos, target, smooth_t(smoothing, dt));
  if (c->min.x != c->max.x || c->min.y != c->max.y) {
    float hw = c->view_w * 0.5f / c->zoom, hh = c->view_h * 0.5f / c->zoom;
    c->pos.x = clampf(c->pos.x, c->min.x + hw, c->max.x - hw);
    c->pos.y = clampf(c->pos.y, c->min.y + hh, c->max.y - hh);
  }
}
