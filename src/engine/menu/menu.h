#pragma once
#include "engine.h"
#include "text.h"

/** Maximum items in one Menu; menu_add beyond it is ignored. Split longer lists into pages or sub-menus.
 *  @see Menu, menu_add
 */
#define MENU_MAX_ITEMS 12

/** One entry in a Menu: a label and the function to call when it is chosen.
 *  @field label text shown; must outlive the menu (a literal or a static buffer)
 *  @field action called with the Engine when chosen; NULL for a label that does nothing
 *  @see Menu, menu_add
 */
typedef struct { const char *label; void (*action)(Engine *); } MenuItem;

/** A vertical list of selectable text items with keyboard, gamepad and mouse handling built in: the title screen, pause menu, options, a shop, a level select. Keep one in a scene static, build it in on_enter (menu_init, menu_add..., menu_center), feed it input in handle_input (menu_handle_input) and draw it in render (menu_render). Actions are plain functions taking the Engine, so "Play" is a function that calls engine_replace(e, game_scene()). For items whose text changes (a volume percentage) point the label at a static char buffer you snprintf into.
 *  @field items the entries, in order
 *  @field count number of items, up to MENU_MAX_ITEMS (12)
 *  @field selected index of the highlighted item; set it to preselect
 *  @field x left edge of the label column in logical pixels
 *  @field y top of the first item
 *  @field line_h vertical distance between items; menu_init sets it from the font (line height + 6); 0 = from font
 *  @field hit_w width of the clickable strip per item; 0 = 200. menu_center() sets it to the widest label.
 *  @field font font for the labels; a zero Font is the debug font
 *  @field color color of unselected items; alpha 0 = default grey
 *  @field color_selected color of the highlighted item; alpha 0 = default yellow
 *  @see menu_init, menu_add, menu_center, menu_handle_input, menu_render, MenuItem, gge-menu
 */
typedef struct Menu {
  MenuItem items[MENU_MAX_ITEMS];
  int   count, selected;
  float x, y, line_h;      /* line_h 0 = from font */
  float hit_w;             /* clickable width; 0 = 200 */
  Font  font;              /* zero = debug font */
  Color color, color_selected;       /* zero = defaults */
} Menu;

/** Reset a Menu: no items, selection 0, position x,y, font (a zero Font uses the debug font), default colors (grey, yellow when selected), line height from the font plus 6, and a 200-pixel click width. Call in on_enter before menu_add; calling it again rebuilds the menu from scratch, which is how a dynamic menu (a shop whose items depend on GameState) is refreshed.
 *  @see menu_add, menu_center, Menu
 */
void menu_init(Menu *m, float x, float y, Font font);
/** Append an item. label must outlive the menu (a string literal or static buffer). action is called with the Engine when the item is chosen; NULL is allowed for a label that does nothing (a heading, a disabled entry). Up to 12 items; extra calls are ignored.
 *  @see menu_init, menu_handle_input, MenuItem
 */
void menu_add(Menu *m, const char *label, void (*action)(Engine *));
/** Move the selection with up/down arrows, W/S, or the dpad (wrapping at both ends); hovering the mouse over an item selects it. Activate the selected item with Enter, space, or PAD_SOUTH, or click an item with the left button. The chosen item's action runs immediately, inside this call, so an action that pushes or pops a scene takes effect after the current callback like any other stack change. Call from the owning scene's handle_input.
 *  @see menu_render, menu_add
 */
void menu_handle_input(Menu *m, Engine *e, const Input *in);
/** Draw the items top to bottom from m->x, m->y, line_h apart, indenting the selected one by 12 pixels and drawing it in color_selected. Draw a backdrop and title first, then call this, from the owning scene's render.
 *  @see menu_handle_input, menu_center
 */
void menu_render(const Menu *m);
/** Position the menu block in the middle of the logical screen based on its widest label and item count, and set hit_w to match. Call after the last menu_add (and again if labels change length). For a menu that should sit elsewhere, set x and y directly instead.
 *  @see menu_init, menu_add
 */
void menu_center(Menu *m, Engine *e);
