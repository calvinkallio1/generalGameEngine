#pragma once
#include "engine.h"

/** A scrolling, selectable grid of equal-sized cells: an inventory, a party roster, a level select, a shop. The grid owns layout and the cursor only; what each cell shows is the game's business, drawn into the box grid_cell_rect returns. Items are numbered 0..count-1 and laid out left to right, top to bottom, cols per row. When there are more rows than fit in the window the extra ones scroll, and the cursor always stays visible (scroll follows selection). Keep one in a scene static, set it up in on_enter (grid_init, grid_set_count), feed it input in handle_input (grid_handle_input) and draw it in render (grid_render, then your cell contents).
 *  @field x left edge of the window in logical pixels
 *  @field y top edge of the window
 *  @field cell pitch of one cell: its size plus the gap to the next
 *  @field gap space between cells, taken out of cell; grid_init sets 4. 0 = cells touch.
 *  @field cols cells per row
 *  @field rows rows that fit in the window; more items than cols*rows scroll
 *  @field count items in the grid; set with grid_set_count, never directly
 *  @field selected cursor as an item index; -1 while the grid is empty
 *  @field scroll first visible row
 *  @field wrap true: the cursor wraps at both ends like a Menu (fine for a fixed grid such as an inventory). false, the default: it stops at the edges, which reads better for a scrolling list.
 *  @field color background of every visible cell in grid_render; alpha 0 = default dark
 *  @field color_selected outline of the selected cell in grid_render; alpha 0 = default yellow
 *  @see grid_init, grid_set_count, grid_handle_input, grid_render, grid_cell_rect, gge-grid
 */
typedef struct Grid {
  float x, y;                     /* window top-left */
  float cell, gap;                /* pitch, spacing; gap 0 = touching */
  int   cols, rows;               /* cells across, visible rows down */
  int   count;                    /* items; rows beyond the window scroll */
  int   selected;                 /* item index; -1 when empty */
  int   scroll;                   /* first visible row */
  bool  wrap;                     /* cursor wraps at the ends (default: clamps) */
  Color color, color_selected;    /* zero = defaults */
} Grid;

/** Reset a Grid: window at x,y, cells cell pixels apart with a 4-pixel gap, cols across and rows visible, no items, selection -1, clamping cursor, default colors. Call in on_enter, then grid_set_count with the number of items; calling it again rebuilds the grid from scratch.
 *  @see grid_set_count, Grid
 */
void grid_init(Grid *g, float x, float y, float cell, int cols, int rows);
/** Tell the grid how many items it holds. Clamps selected into range (-1 when empty, otherwise 0..count-1) and scroll so the selection stays visible. Call after grid_init and again whenever the underlying list changes length, so a stale cursor can never point past the end.
 *  @see grid_init, grid_select
 */
void grid_set_count(Grid *g, int count);
/** Rows the items occupy in total, visible or not: count divided by cols, rounded up.
 *  @return 0 when the grid is empty
 *  @see grid_max_scroll
 */
int  grid_total_rows(const Grid *g);
/** The largest scroll value that still fills the window: total rows minus visible rows, never below 0. scroll is kept within 0..this.
 *  @see grid_scroll_by, grid_total_rows
 */
int  grid_max_scroll(const Grid *g);
/** The window rectangle on screen: cols*cell wide and rows*cell tall from x,y, including the trailing gap. Draw a panel behind it, or hit-test it to see whether the mouse is over the grid at all.
 *  @see grid_cell_rect
 */
Rect grid_window(const Grid *g);
/** Screen rectangle of item i with the current scroll applied and the gap removed, so it is the box to draw the item into. Defined for any i, even one scrolled out of the window; use grid_visible or the grid_first_visible..grid_end_visible range to draw only what shows.
 *  @see grid_visible, grid_first_visible, grid_end_visible
 */
Rect grid_cell_rect(const Grid *g, int i);
/** True if item i exists and its row is inside the window at the current scroll.
 *  @see grid_cell_rect
 */
bool grid_visible(const Grid *g, int i);
/** Index of the first item inside the window. Together with grid_end_visible this is the render loop: for (i = grid_first_visible(g); i < grid_end_visible(g); i++) draw at grid_cell_rect(g, i).
 *  @see grid_end_visible, grid_cell_rect
 */
int  grid_first_visible(const Grid *g);
/** One past the last item inside the window, capped at count.
 *  @see grid_first_visible
 */
int  grid_end_visible(const Grid *g);
/** The visible item whose cell contains the screen point mx,my.
 *  @return the item index, or -1 when the point is on no cell (the gaps between cells count as no cell)
 *  @see grid_handle_input, grid_window
 */
int  grid_hit(const Grid *g, float mx, float my);
/** Move the cursor to item i, clamped into 0..count-1, and scroll the window so it is visible. The way to preselect an item or to restore the cursor after rebuilding the list.
 *  @see grid_move, grid_set_count
 */
void grid_select(Grid *g, int i);
/** Move the cursor by dx cells sideways and dy rows down (negative is left or up). Sideways moves run on through the ends of rows, so right from the last column lands on the next row's first. A downward move past the last row lands on the last item; upward from the first row stays put. With wrap set, moves wrap around both ends instead. The window scrolls to keep the cursor visible.
 *  @see grid_select, grid_handle_input
 */
void grid_move(Grid *g, int dx, int dy);
/** Scroll the window by rows (negative is up), clamped to 0..grid_max_scroll. The cursor does not move, so it may leave the window; the next cursor move brings it back.
 *  @see grid_max_scroll, grid_move
 */
void grid_scroll_by(Grid *g, int rows);
/** Move the cursor with the arrows, WASD or the dpad; scroll with the mouse wheel; hovering the mouse over a visible cell selects it. Call from the owning scene's handle_input. Nothing happens while the grid is empty.
 *  @return true on the frame the selected item is activated: Enter, space, PAD_SOUTH, or a left click on a cell. The scene then acts on g->selected (feed the cat, equip the item, open the level).
 *  @see grid_render, grid_hit, grid_move
 */
bool grid_handle_input(Grid *g, const Input *in);
/** Draw the background of every visible cell, a grey outline round each, and the selected one outlined in color_selected. Call from render after the backdrop and before drawing the cell contents at grid_cell_rect; skip it and draw your own cells if the look does not suit.
 *  @see grid_render_scrollbar, grid_cell_rect
 */
void grid_render(const Grid *g);
/** A track along the window's right edge, w pixels wide, with a thumb showing where the window sits in the list. Draws nothing when every row already fits, so it is safe to call unconditionally.
 *  @see grid_render, grid_max_scroll
 */
void grid_render_scrollbar(const Grid *g, float w);
