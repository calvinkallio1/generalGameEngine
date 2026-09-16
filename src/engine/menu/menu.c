#include "menu.h"

void menu_init(Menu *m, float x, float y, Font font) {
  *m = (Menu){ .x = x, .y = y, .font = font };
  m->line_h = text_line_height(font) + 6;
  m->hit_w = 200;
  m->color = (Color){ 160, 160, 160, 255 };
  m->color_selected = (Color){ 255, 220, 80, 255 };
}

void menu_add(Menu *m, const char *label, void (*action)(Engine *)) {
  if (m->count < MENU_MAX_ITEMS) m->items[m->count++] = (MenuItem){ label, action };
}

static bool hit(const Menu *m, int i, float mx, float my) {
  Rect box = { m->x, m->y + i * m->line_h - 2, m->hit_w > 0 ? m->hit_w : 200, m->line_h };
  Point p = { mx, my };
  return SDL_PointInRectFloat(&p, &box);
}

void menu_handle_input(Menu *m, Engine *e, const Input *in) {
  if (m->count == 0) return;
  bool down = key_pressed(in, KEY_DOWN) || key_pressed(in, KEY_S) || pad_pressed(in, PAD_DPAD_DOWN);
  bool up   = key_pressed(in, KEY_UP)   || key_pressed(in, KEY_W) || pad_pressed(in, PAD_DPAD_UP);
  if (down) m->selected = (m->selected + 1) % m->count;
  if (up)   m->selected = (m->selected + m->count - 1) % m->count;

  for (int i = 0; i < m->count; ++i) {
    if (hit(m, i, in->mouse_x, in->mouse_y)) {
      m->selected = i;
      if (mouse_pressed(in, MOUSE_LEFT)) { if (m->items[i].action) m->items[i].action(e); return; }
    }
  }
  bool confirm = key_pressed(in, KEY_ENTER) || key_pressed(in, KEY_SPACE) || pad_pressed(in, PAD_SOUTH);
  if (confirm && m->items[m->selected].action) m->items[m->selected].action(e);
}

void menu_render(const Menu *m) {
  for (int i = 0; i < m->count; ++i) {
    bool sel = (i == m->selected);
    Color c = sel ? m->color_selected : m->color;
    if (c.a == 0) c = sel ? (Color){ 255, 220, 80, 255 } : (Color){ 160, 160, 160, 255 };
    text_draw(m->font, m->x + (sel ? 12 : 0), m->y + i * m->line_h, c, m->items[i].label);
  }
}

void menu_center(Menu *m, Engine *e) {
  int w = 0, h, widest = 0;
  for (int i = 0; i < m->count; i++) {
    text_size(m->font, m->items[i].label, &w, &h);
    if (w > widest) {
      widest = w;
    }
  }
  float block_h = m->count * m->line_h;
  m->x = (e->logical_w - widest) / 2.0f;
  m->y = (e->logical_h - block_h) / 2.0f;
  m->hit_w = widest + 12;
}