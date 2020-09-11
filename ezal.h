/*
MIT License

Copyright (c) 2020 Richard Marks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef EZAL_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#ifndef EZAL_MAX_USER_DATA_PTRS
#define EZAL_MAX_USER_DATA_PTRS 1
#endif

struct EZALAllegroContext {
  ALLEGRO_TIMER* timer;
  ALLEGRO_DISPLAY* display;
  ALLEGRO_BITMAP* buffer;
  ALLEGRO_FONT* font;
  ALLEGRO_EVENT_QUEUE* event_queue;
  ALLEGRO_EVENT event;
  ALLEGRO_COLOR screen_color;
  ALLEGRO_COLOR border_color;
};

struct EZALConfig {
  int width;
  int height;
  int logical_width;
  int logical_height;
  int audio_samples;
  int frame_rate;

  bool fullscreen;
  bool auto_scale;
  bool stretch_scale;
  bool enable_audio;
  bool enable_mouse;
  bool enable_keyboard;
  bool debug;
};

struct EZALInputContext {
  unsigned char key[ALLEGRO_KEY_MAX];
  unsigned char last_key;

  unsigned char mouse_state;
  unsigned int mouse_x;
  unsigned int mouse_y;
  unsigned int mouse_button;

  int relative_mouse_x;
  int relative_mouse_y;
};

struct EZALRuntimeContext {
  struct EZALConfig* cfg;
  struct EZALAllegroContext* al_ctx;
  struct EZALInputContext* input;

  bool is_running;
  bool should_redraw;

  void (*create)(struct EZALRuntimeContext*);
  void (*destroy)(struct EZALRuntimeContext*);
  void (*update)(struct EZALRuntimeContext*);
  void (*render)(struct EZALRuntimeContext*);
  void (*post_render)(struct EZALRuntimeContext*);

  void* user[EZAL_MAX_USER_DATA_PTRS];
  void* _ezal_reserved;
};

typedef void (*EZALFPTR)(struct EZALRuntimeContext*);

struct EZALRuntimeAdapter {
  struct EZALRuntimeContext* rt_ctx;
  int (*start)(struct EZALRuntimeAdapter*);
};

#define EZAL_FN(identifier) void identifier(struct EZALRuntimeContext* ctx)
#define EZAL_KEY(keycode) (ctx->input->key[keycode] != 0x0)

extern void ezal_use_config_defaults(struct EZALConfig* cfg);

extern int ezal_start(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg
);

extern struct EZALRuntimeAdapter* ezal_init(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg);

extern void ezal_stop(struct EZALRuntimeContext* ctx);

#define EZAL_H
#endif // !EZAL_H
