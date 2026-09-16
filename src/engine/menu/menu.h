#pragma once
#include "engine.h"
#include "text.h"

#define MENU_MAX_ITEMS 12

typedef struct { const char *label; void (*action)(Engine *); } MenuItem;

typedef struct Menu {
  MenuItem items[MENU_MAX_ITEMS];
  int   count, selected;
  float x, y, line_h;      /* line_h 0 = from font */
  float hit_w;             /* clickable width; 0 = 200 */
  Font  font;              /* zero = debug font */
  Color color, color_selected;       /* zero = defaults */
} Menu;

void menu_init(Menu *m, float x, float y, Font font);
void menu_add(Menu *m, const char *label, void (*action)(Engine *));
void menu_handle_input(Menu *m, Engine *e, const Input *in);
void menu_render(const Menu *m);
void menu_center(Menu *m, Engine *e);