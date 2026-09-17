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

/** Reset a Menu: no items, selection 0, position x,y, font (a zero Font uses the debug font), default colors and line height.
 *  @see menu_add, menu_center
 */
void menu_init(Menu *m, float x, float y, Font font);
/** Append an item. label must outlive the menu (a string literal or static buffer). action is called with the Engine when the item is chosen; NULL is allowed for a label that does nothing. Up to 12 items.
 *  @see menu_init, menu_handle_input
 */
void menu_add(Menu *m, const char *label, void (*action)(Engine *));
/** Move the selection with up/down keys, dpad, or mouse hover; activate with Enter, space, pad south, or a click. Call from the owning scene's handle_input.
 *  @see menu_render
 */
void menu_handle_input(Menu *m, Engine *e, const Input *in);
/** Draw the items top to bottom from m->x, m->y, indenting and recoloring the selected one. Call from the owning scene's render.
 *  @see menu_handle_input
 */
void menu_render(const Menu *m);
/** Position the menu block in the middle of the logical screen based on its widest label and item count. Call after the last menu_add.
 *  @see menu_init
 */
void menu_center(Menu *m, Engine *e);