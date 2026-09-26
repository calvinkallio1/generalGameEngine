#include "grid.h"
#include "draw.h"

#define GRID_DEFAULT_GAP 4.0f

static const Color GRID_BG       = { 20, 20, 30, 220 };
static const Color GRID_OUTLINE  = { 160, 160, 160, 255 };
static const Color GRID_SELECTED = { 255, 220, 80, 255 };

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void grid_init(Grid *g, float x, float y, float cell_w, float cell_h, int cols, int rows) {
  *g = (Grid){ .x = x, .y = y, .cell_w = cell_w, .cell_h = cell_h, .gap = GRID_DEFAULT_GAP,
               .cols = cols > 0 ? cols : 1, .rows = rows > 0 ? rows : 1, .selected = -1 };
  g->color          = GRID_BG;
  g->color_selected = GRID_SELECTED;
}

int grid_total_rows(const Grid *g) { return (g->count + g->cols - 1) / g->cols; }

int grid_max_scroll(const Grid *g) {
  int m = grid_total_rows(g) - g->rows;
  return m > 0 ? m : 0;
}

/* keep scroll in range and, if there is a cursor, the window over it */
static void settle(Grid *g) {
  g->scroll = clampi(g->scroll, 0, grid_max_scroll(g));
  if (g->selected < 0) return;
  int row = g->selected / g->cols;
  if (row < g->scroll)            g->scroll = row;
  if (row >= g->scroll + g->rows) g->scroll = row - g->rows + 1;
}

void grid_set_count(Grid *g, int count) {
  g->count = count > 0 ? count : 0;
  g->selected = g->count == 0 ? -1 : clampi(g->selected, 0, g->count - 1);
  settle(g);
}

Rect grid_window(const Grid *g) {
  return rect(g->x, g->y, (float)g->cols * g->cell_w, (float)g->rows * g->cell_h);
}

Rect grid_cell_rect(const Grid *g, int i) {
  int col = i % g->cols, row = i / g->cols - g->scroll;
  return rect(g->x + (float)col * g->cell_w, g->y + (float)row * g->cell_h, g->cell_w - g->gap, g->cell_h - g->gap);
}

bool grid_visible(const Grid *g, int i) {
  if (i < 0 || i >= g->count) return false;
  int row = i / g->cols;
  return row >= g->scroll && row < g->scroll + g->rows;
}

int grid_first_visible(const Grid *g) {
  int i = g->scroll * g->cols;
  return i < g->count ? i : g->count;
}

int grid_end_visible(const Grid *g) {
  int i = (g->scroll + g->rows) * g->cols;
  return i < g->count ? i : g->count;
}

int grid_hit(const Grid *g, float mx, float my) {
  Point p = { mx, my };
  for (int i = grid_first_visible(g); i < grid_end_visible(g); i++) {
    Rect r = grid_cell_rect(g, i);
    if (SDL_PointInRectFloat(&p, &r)) return i;
  }
  return -1;
}

void grid_select(Grid *g, int i) {
  g->selected = g->count == 0 ? -1 : clampi(i, 0, g->count - 1);
  settle(g);
}

void grid_move(Grid *g, int dx, int dy) {
  if (g->count == 0) return;
  int i = g->selected < 0 ? 0 : g->selected;
  if (g->wrap) {
    i = ((i + dx + dy * g->cols) % g->count + g->count) % g->count;
  } else {
    i = clampi(i + dx, 0, g->count - 1);              /* sideways runs on through row ends */
    int target = i + dy * g->cols;
    if (target < 0)              target = i;          /* already on the top row */
    else if (target >= g->count) target = (i / g->cols == grid_total_rows(g) - 1) ? i : g->count - 1;
    i = target;
  }
  grid_select(g, i);
}

void grid_scroll_by(Grid *g, int rows) {
  g->scroll = clampi(g->scroll + rows, 0, grid_max_scroll(g));
}

bool grid_handle_input(Grid *g, const Input *in) {
  if (g->count == 0) return false;

  int dx = 0, dy = 0;
  if (key_pressed(in, KEY_RIGHT) || key_pressed(in, KEY_D) || pad_pressed(in, PAD_DPAD_RIGHT)) dx++;
  if (key_pressed(in, KEY_LEFT)  || key_pressed(in, KEY_A) || pad_pressed(in, PAD_DPAD_LEFT))  dx--;
  if (key_pressed(in, KEY_DOWN)  || key_pressed(in, KEY_S) || pad_pressed(in, PAD_DPAD_DOWN))  dy++;
  if (key_pressed(in, KEY_UP)    || key_pressed(in, KEY_W) || pad_pressed(in, PAD_DPAD_UP))    dy--;
  if (dx || dy) grid_move(g, dx, dy);

  if (in->wheel != 0) {                               /* wheel up (positive) = towards the top */
    int notches = (int)(in->wheel < 0 ? -in->wheel : in->wheel);
    if (notches < 1) notches = 1;
    grid_scroll_by(g, in->wheel > 0 ? -notches : notches);
  }

  int hover = grid_hit(g, in->mouse_x, in->mouse_y);
  if (hover >= 0) {
    g->selected = hover;                              /* visible by construction: no settle needed */
    if (mouse_pressed(in, MOUSE_LEFT)) return true;
  }
  return key_pressed(in, KEY_ENTER) || key_pressed(in, KEY_SPACE) || pad_pressed(in, PAD_SOUTH);
}

void grid_render(const Grid *g) {
  Color bg  = g->color.a          ? g->color          : GRID_BG;
  Color sel = g->color_selected.a ? g->color_selected : GRID_SELECTED;
  for (int i = grid_first_visible(g); i < grid_end_visible(g); i++) {
    Rect r = grid_cell_rect(g, i);
    draw_rect(r, bg);
    draw_rect_outline(r, i == g->selected ? sel : GRID_OUTLINE);
  }
}

void grid_render_scrollbar(const Grid *g, float w) {
  int total = grid_total_rows(g);
  if (total <= g->rows) return;
  Rect win = grid_window(g);
  Rect track = rect(win.x + win.w, win.y, w, win.h - g->gap);   /* one gap right of the last column */
  float thumb_h = track.h * (float)g->rows   / (float)total;
  float thumb_y = track.y + track.h * (float)g->scroll / (float)total;
  draw_rect(track, GRID_BG);
  draw_rect(rect(track.x, thumb_y, w, thumb_h), GRID_OUTLINE);
}
