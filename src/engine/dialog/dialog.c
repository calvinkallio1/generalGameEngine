#include "dialog.h"
#include "draw.h"
#include <string.h>
#include <stdio.h>

#define MAX_LINES 8
#define LINE_MAX  128

void dialog_show(Dialog *d, const char *speaker, const char *text) {
  d->speaker = speaker; d->text = text; d->chars_shown = 0;
  if (d->chars_per_sec <= 0) d->chars_per_sec = 40;
  d->active = true;
}

void dialog_update(Dialog *d, const Input *in, float dt) {
  if (!d->active || !d->text) return;
  size_t len = strlen(d->text);
  d->chars_shown += d->chars_per_sec * dt;
  bool advance = key_pressed(in, KEY_SPACE) || key_pressed(in, KEY_ENTER)
              || mouse_pressed(in, MOUSE_LEFT) || pad_pressed(in, PAD_SOUTH);
  if (advance) {
    if (d->chars_shown < (float)len) d->chars_shown = (float)len;
    else d->active = false;
  }
}

static int wrap(Font font, const char *text, float max_w, char out[MAX_LINES][LINE_MAX]) {
  int lines = 0; char line[LINE_MAX] = ""; const char *p = text;
  while (*p && lines < MAX_LINES) {
    const char *end = p; while (*end && *end != ' ') ++end;
    char word[64]; snprintf(word, sizeof word, "%.*s", (int)(end - p), p);
    char trial[LINE_MAX]; snprintf(trial, sizeof trial, "%s%s%s", line, *line ? " " : "", word);
    int w = 0, h = 0; text_size(font, trial, &w, &h);
    if ((float)w > max_w && *line) { snprintf(out[lines++], LINE_MAX, "%s", line); snprintf(line, sizeof line, "%s", word); }
    else snprintf(line, sizeof line, "%s", trial);
    p = *end ? end + 1 : end;
  }
  if (*line && lines < MAX_LINES) snprintf(out[lines++], LINE_MAX, "%s", line);
  return lines;
}

void dialog_render(Dialog *d, Engine *e) {
  if (!d->active || !d->text) return;
  float lh = text_line_height(d->font) + 2;
  float pad = 8, box_h = lh * 3 + 16 + (d->speaker ? lh : 0);
  Rect box = { pad, e->logical_h - box_h - pad, e->logical_w - 2 * pad, box_h };

  draw_rect(box, rgba(0, 0, 0, 200));
  draw_rect_outline(box, COLOR_WHITE);

  Color white = COLOR_WHITE, gold = COLOR_YELLOW;
  float y = box.y + 8;
  if (d->speaker) { text_draw(d->font, box.x + 8, y, gold, d->speaker); y += lh; }

  char shown[512]; snprintf(shown, sizeof shown, "%.*s", (int)d->chars_shown, d->text);
  char lines[MAX_LINES][LINE_MAX]; int n = wrap(d->font, shown, box.w - 16, lines);
  for (int i = 0; i < n && i < 3; ++i) { text_draw(d->font, box.x + 8, y, white, lines[i]); y += lh; }
}
