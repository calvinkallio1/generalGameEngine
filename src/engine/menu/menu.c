#include "menu.h"

void menu_init(Menu *m, float x, float y, Font font) {
  *m = (Menu){ .x = x, .y = y, .font = font };
  m->line_h = text_line_height(font) + 6;
  m->hit_w = 200;
  m->color = (SDL_Color){ 160, 160, 160, 255 };
  m->color_selected = (SDL_Color){ 255, 220, 80, 255 };
}

void menu_add(Menu *m, const char *label, void (*action)(Engine *)) {
  if (m->count < MENU_MAX_ITEMS) m->items[m->count++] = (MenuItem){ label, action };
}

static bool hit(const Menu *m, int i, float mx, float my) {
  SDL_FRect box = { m->x, m->y + i * m->line_h - 2, m->hit_w > 0 ? m->hit_w : 200, m->line_h };
  SDL_FPoint p = { mx, my };
  return SDL_PointInRectFloat(&p, &box);
}

void menu_handle_input(Menu *m, Engine *e, const Input *in) {
  if (m->count == 0) return;
  bool down = key_pressed(in, SDL_SCANCODE_DOWN) || key_pressed(in, SDL_SCANCODE_S) || pad_pressed(in, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  bool up   = key_pressed(in, SDL_SCANCODE_UP)   || key_pressed(in, SDL_SCANCODE_W) || pad_pressed(in, SDL_GAMEPAD_BUTTON_DPAD_UP);
  if (down) m->selected = (m->selected + 1) % m->count;
  if (up)   m->selected = (m->selected + m->count - 1) % m->count;

  for (int i = 0; i < m->count; ++i) {
    if (hit(m, i, in->mouse_x, in->mouse_y)) {
      m->selected = i;
      if (mouse_pressed(in, SDL_BUTTON_LEFT)) { if (m->items[i].action) m->items[i].action(e); return; }
    }
  }
  bool confirm = key_pressed(in, SDL_SCANCODE_RETURN) || key_pressed(in, SDL_SCANCODE_SPACE) || pad_pressed(in, SDL_GAMEPAD_BUTTON_SOUTH);
  if (confirm && m->items[m->selected].action) m->items[m->selected].action(e);
}

void menu_render(const Menu *m, SDL_Renderer *r) {
  for (int i = 0; i < m->count; ++i) {
    bool sel = (i == m->selected);
    SDL_Color c = sel ? m->color_selected : m->color;
    if (c.a == 0) c = sel ? (SDL_Color){ 255, 220, 80, 255 } : (SDL_Color){ 160, 160, 160, 255 };
    text_draw(r, m->font, m->x + (sel ? 12 : 0), m->y + i * m->line_h, c, m->items[i].label);
  }
}
