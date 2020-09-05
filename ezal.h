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
/*

#include "ezal.h"

int main(int argc, char* argv[])
{
  struct EZALConfig cfg;

  ezal_use_config_defaults(&cfg);

  cfg.width = 1280; // default 800
  cfg.height = 600; // default 600
  cfg.logical_width = 320; // default 800
  cfg.logical_height = 200; // default 600
  cfg.fullscreen = false; // default false
  cfg.auto_scale = true; // default false
  cfg.stretch_scale = false; // default false
  cfg.audio_samples = 16; // default 1
  cfg.enable_audio = true; // default true
  cfg.enable_mouse = true; // default true
  cfg.enable_keyboard = true; // default true
  cfg.debug = true; // default false
  cfg.frame_rate = 30; // default 30

  return ezal_start(
    "My Game Title",
    &my_create_fn,
    &my_destroy_fn,
    &my_update_fn,
    &my_render_fn,
    &cfg);
}

*/
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
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
  unsigned char key[ALLEGRO_KEY_MAX];

  float mouseX;
  float mouseY;
  unsigned int mouseButton;
  unsigned char mouseState;
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

struct EZALRuntimeContext {
  struct EZALConfig* cfg;
  struct EZALAllegroContext* al_ctx;

  bool is_running;
  bool is_fullscreen;
  bool should_redraw;
  bool did_tick;

  void (*create)(struct EZALRuntimeContext*);
  void (*destroy)(struct EZALRuntimeContext*);
  void (*update)(struct EZALRuntimeContext*);
  void (*render)(struct EZALRuntimeContext*);

  void* user[EZAL_MAX_USER_DATA_PTRS];
};

typedef void (*EZALFPTR)(struct EZALRuntimeContext*);

#define EZAL_FN(identifier) void identifier(struct EZALRuntimeContext* ctx)
#define EZAL_KEY(keycode) (ctx->al_ctx->key[keycode] != 0x0)

extern void ezal_use_config_defaults(struct EZALConfig* cfg);

extern int ezal_start(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg
);

extern void ezal_stop(struct EZALRuntimeContext* ctx);

#define EZAL_H
#endif // !EZAL_H
