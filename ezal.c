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

#include "ezal.h"

// private data structures

struct EZALPrivateData {
  struct EZALConfig cfg;
  struct EZALAllegroContext al_ctx;
  struct EZALRuntimeContext rt_ctx;
  struct EZALInputContext input;

  int x;
  int y;
  int w;
  int h;

  void (*update)(struct EZALPrivateData*);
  void (*render)(struct EZALPrivateData*);
  void (*present)(struct EZALPrivateData*);
  void (*halt)(struct EZALPrivateData*);
  void (*resize)(struct EZALPrivateData*);
};

// ezal private function pointer targets
void ezal_private_halt(struct EZALPrivateData* pd)
{
  pd->rt_ctx.is_running = false;
  pd->rt_ctx.should_redraw = false;
}

void ezal_private_resize(struct EZALPrivateData* pd)
{
  int display_width = al_get_display_width(pd->al_ctx.display);
  int display_height = al_get_display_height(pd->al_ctx.display);

  pd->cfg.width = display_width;
  pd->cfg.height = display_height;

  if (pd->cfg.stretch_scale)
  {
    pd->x = 0;
    pd->y = 0;
    pd->w = display_width;
    pd->h = display_height;
  }
  else
  {
    float ratio = (float)display_width / (float)pd->cfg.logical_width;

    if (display_height < ((float)pd->cfg.logical_height * ratio))
    {
      ratio = (float)display_height / (float)pd->cfg.logical_height;
    }

    pd->w = pd->cfg.logical_width * ratio;
    pd->h = pd->cfg.logical_height * ratio;
    pd->x = (display_width - pd->w) * 0.5f;
    pd->y = (display_height - pd->h) * 0.5f;
  }
}

void ezal_private_update(struct EZALPrivateData* pd)
{
  al_wait_for_event(pd->al_ctx.event_queue, &pd->al_ctx.event);

  switch (pd->al_ctx.event.type)
  {
    case ALLEGRO_EVENT_TIMER: {
      pd->rt_ctx.should_redraw = true;

      pd->rt_ctx.update(&pd->rt_ctx);

      for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
        pd->input.key[i] &= 1;
      }
      pd->input.mouse_state &= 1;
    } break;
    case ALLEGRO_EVENT_KEY_DOWN: {
      pd->input.key[pd->al_ctx.event.keyboard.keycode] = 1 | 2;
    } break;
    case ALLEGRO_EVENT_KEY_UP: {
      pd->input.key[pd->al_ctx.event.keyboard.keycode] &= 2;
    } break;
    case ALLEGRO_EVENT_MOUSE_AXES: {
      pd->input.mouse_x = pd->al_ctx.event.mouse.x;
      pd->input.mouse_y = pd->al_ctx.event.mouse.y;
    } break;
    case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
      pd->input.mouse_state = 1 | 2;
      pd->input.mouse_button = pd->al_ctx.event.mouse.button;
    } break;
    case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
      pd->input.mouse_state &= 2;
      pd->input.mouse_button = pd->al_ctx.event.mouse.button;
    } break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE: {
      pd->halt(pd);
    } break;
    case ALLEGRO_EVENT_DISPLAY_RESIZE: {
      if (al_acknowledge_resize(pd->al_ctx.event.display.source))
      {
        pd->resize(pd);
      }
    } break;
    case ALLEGRO_EVENT_DISPLAY_SWITCH_IN: {
      pd->resize(pd);
    } break;
    default: break;
  }
}

void ezal_private_render_default(struct EZALPrivateData* pd)
{
  al_clear_to_color(pd->al_ctx.screen_color);
}

void ezal_private_present_default(struct EZALPrivateData* pd)
{
  al_flip_display();
}

void ezal_private_render_scaled(struct EZALPrivateData* pd)
{
  al_set_target_bitmap(pd->al_ctx.buffer);
  al_clear_to_color(pd->al_ctx.screen_color);
}

void ezal_private_present_scaled(struct EZALPrivateData* pd)
{
  al_set_target_backbuffer(pd->al_ctx.display);
  // rgb(51 102 153)
  al_clear_to_color(pd->al_ctx.border_color);
  // al_clear_to_color(al_map_rgb(0, 0, 0));
  al_draw_scaled_bitmap(
    pd->al_ctx.buffer,
    0,
    0,
    pd->cfg.logical_width,
    pd->cfg.logical_height,
    pd->x,
    pd->y,
    pd->w,
    pd->h,
    0);
  al_flip_display();
}

// private api functions

// configuration
void ezal_private_copy_config(struct EZALConfig* src, struct EZALConfig* dst)
{
  ezal_use_config_defaults(dst);

  if (src == 0)
  {
    return;
  }

  memcpy(dst, src, sizeof(struct EZALConfig));
}

// initialization

bool ezal_private_init_allegro(struct EZALPrivateData* pd)
{
  if (!al_init())
  {
    fprintf(stderr, "al_init failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_init\n"); }

  if (pd->cfg.enable_keyboard)
  {
    memset(pd->input.key, 0, sizeof(pd->input.key));
    if (!al_install_keyboard())
    {
      fprintf(stderr, "al_install_keyboard failed.\n");
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_install_keyboard\n"); }
  }

  if (pd->cfg.enable_mouse)
  {
    if (!al_install_mouse())
    {
      fprintf(stderr, "al_install_mouse failed.\n");
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_install_mouse\n"); }
  }

  if (pd->cfg.enable_audio)
  {
    if (!al_install_audio())
    {
      fprintf(stderr, "al_install_audio failed.\n");
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_install_audio\n"); }
  }

  double frame_rate = 1.0 / (double)pd->cfg.frame_rate;

  pd->al_ctx.timer = 0;
  ALLEGRO_TIMER* timer = al_create_timer(frame_rate);
  if (!timer)
  {
    fprintf(stderr, "al_create_timer(%g) failed.\n", frame_rate);
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_create_timer(%g)\n", frame_rate); }
  pd->al_ctx.timer = timer;

  pd->al_ctx.event_queue = 0;
  ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
  if (!event_queue)
  {
    fprintf(stderr, "al_create_event_queue failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_create_event_queue\n"); }
  pd->al_ctx.event_queue = event_queue;

  al_set_new_display_flags(pd->cfg.fullscreen
    ? ALLEGRO_FULLSCREEN_WINDOW
    : ALLEGRO_RESIZABLE);

  pd->al_ctx.display = 0;
  ALLEGRO_DISPLAY* display = al_create_display(
      pd->cfg.width,
      pd->cfg.height);
  if (!display)
  {
    fprintf(stderr, "al_create_display(%d,%d) failed.\n",
        pd->cfg.width,
        pd->cfg.height);
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_create_display(%d,%d)\n", pd->cfg.width, pd->cfg.height); }
  pd->al_ctx.display = display;
  pd->w = al_get_display_width(display);
  pd->h = al_get_display_height(display);

  pd->al_ctx.buffer = 0;
  if (pd->cfg.auto_scale)
  {
    ALLEGRO_BITMAP* buffer = al_create_bitmap(
      pd->cfg.logical_width,
      pd->cfg.logical_height);
    if (!buffer)
    {
      fprintf(stderr, "al_create_bitmap(%d,%d) failed.\n",
          pd->cfg.logical_width,
          pd->cfg.logical_height);
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_create_bitmap(%d,%d)\n", pd->cfg.logical_width, pd->cfg.logical_height); }
    pd->al_ctx.buffer = buffer;
  }

  if (!al_init_font_addon())
  {
    fprintf(stderr, "al_init_font_addon failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_init_font_addon\n"); }

  if (!al_init_ttf_addon())
  {
    fprintf(stderr, "al_init_ttf_addon failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_init_ttf_addon\n"); }

  if (!al_init_image_addon())
  {
    fprintf(stderr, "al_init_image_addon failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_init_image_addon\n"); }

  if (!al_init_primitives_addon())
  {
    fprintf(stderr, "al_init_primitives_addon failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_init_primitives_addon\n"); }

  if (pd->cfg.enable_audio)
  {
    if (!al_init_acodec_addon())
    {
      fprintf(stderr, "al_init_acodec_addon failed.\n");
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_init_acodec_addon\n"); }

    if (!al_reserve_samples(pd->cfg.audio_samples))
    {
      fprintf(stderr, "al_reserve_samples(%d) failed.\n",
          pd->cfg.audio_samples);
      return false;
    }
    if (pd->cfg.debug) { fprintf(stdout, "al_reserve_samples(%d)\n", pd->cfg.audio_samples); }
  }

  pd->al_ctx.font = 0;
  ALLEGRO_FONT* font = al_create_builtin_font();
  if (!font)
  {
    fprintf(stderr, "al_create_builtin_font failed.\n");
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "al_create_builtin_font\n"); }
  pd->al_ctx.font = font;

  pd->al_ctx.border_color = al_map_rgb(0, 0, 0);
  pd->al_ctx.screen_color = al_map_rgb(51, 102, 153);

  if (pd->cfg.enable_keyboard)
  {
    al_register_event_source(
      pd->al_ctx.event_queue,
      al_get_keyboard_event_source());
    if (pd->cfg.debug) { fprintf(stdout, "al_register_event_source(keyboard)\n"); }
  }

  if (pd->cfg.enable_mouse)
  {
    al_register_event_source(
      pd->al_ctx.event_queue,
      al_get_mouse_event_source());
    if (pd->cfg.debug) { fprintf(stdout, "al_register_event_source(mouse)\n"); }
  }

  al_register_event_source(
    pd->al_ctx.event_queue,
    al_get_display_event_source(pd->al_ctx.display));
  if (pd->cfg.debug) { fprintf(stdout, "al_register_event_source(display)\n"); }

  al_register_event_source(
    pd->al_ctx.event_queue,
    al_get_timer_event_source(pd->al_ctx.timer));
  if (pd->cfg.debug) { fprintf(stdout, "al_register_event_source(timer)\n"); }

  return true;
}

void ezal_runtime_do_nothing(struct EZALRuntimeContext* ctx)
{

}

bool ezal_private_init_runtime(
  struct EZALPrivateData* pd,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render
)
{
  pd->rt_ctx.cfg = &pd->cfg;
  pd->rt_ctx.al_ctx = &pd->al_ctx;
  pd->rt_ctx.input = &pd->input;

  pd->rt_ctx.create = &ezal_runtime_do_nothing;
  pd->rt_ctx.destroy = &ezal_runtime_do_nothing;
  pd->rt_ctx.update = &ezal_runtime_do_nothing;
  pd->rt_ctx.render = &ezal_runtime_do_nothing;

  if (create)
  {
    pd->rt_ctx.create = create;
    if (pd->cfg.debug) { fprintf(stdout, "using user create function\n"); }
  }

  if (destroy)
  {
    pd->rt_ctx.destroy = destroy;
    if (pd->cfg.debug) { fprintf(stdout, "using user destroy function\n"); }
  }

  if (update)
  {
    pd->rt_ctx.update = update;
    if (pd->cfg.debug) { fprintf(stdout, "using user update function\n"); }
  }

  if (render)
  {
    pd->rt_ctx.render = render;
    if (pd->cfg.debug) { fprintf(stdout, "using user render function\n"); }
  }

  return true;
}

bool ezal_private_init_pd(struct EZALPrivateData* pd)
{
  pd->resize = &ezal_private_resize;
  pd->halt = &ezal_private_halt;
  pd->update = &ezal_private_update;
  pd->render = &ezal_private_render_default;
  pd->present = &ezal_private_present_default;

  if (pd->cfg.auto_scale)
  {
    pd->render = &ezal_private_render_scaled;
    pd->present = &ezal_private_present_scaled;
    if (pd->cfg.debug) { fprintf(stdout, "auto scale enabled\n"); }
  }

  return true;
}

bool ezal_private_init(
  struct EZALPrivateData* pd,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render)
{
  // initialize allegro
  if (!ezal_private_init_allegro(pd))
  {
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "allegro initialization complete\n"); }

  // initialize runtime
  if (!ezal_private_init_runtime(pd, create, destroy, update, render))
  {
    return false;
  }
  if (pd->cfg.debug) { fprintf(stdout, "runtime initialization complete\n"); }

  // initialize pd private function pointers based on cfg
  if (!ezal_private_init_pd(pd))
  {
    return false;
  }

  return true;
}

// shutdown
bool ezal_private_quit(struct EZALPrivateData* pd)
{
  if (pd->al_ctx.font)
  {
    al_destroy_font(pd->al_ctx.font);
    pd->al_ctx.font = 0;
    if (pd->cfg.debug) { fprintf(stdout, "al_destroy_font\n"); }
  }

  if (pd->al_ctx.buffer)
  {
    al_destroy_bitmap(pd->al_ctx.buffer);
    pd->al_ctx.buffer = 0;
    if (pd->cfg.debug) { fprintf(stdout, "al_destroy_bitmap\n"); }
  }

  if (pd->al_ctx.display)
  {
    al_destroy_display(pd->al_ctx.display);
    pd->al_ctx.display = 0;
    if (pd->cfg.debug) { fprintf(stdout, "al_destroy_display\n"); }
  }

  if (pd->al_ctx.timer)
  {
    al_destroy_timer(pd->al_ctx.timer);
    pd->al_ctx.timer = 0;
    if (pd->cfg.debug) { fprintf(stdout, "al_destroy_timer\n"); }
  }

  if (pd->al_ctx.event_queue)
  {
    al_destroy_event_queue(pd->al_ctx.event_queue);
    pd->al_ctx.event_queue = 0;
    if (pd->cfg.debug) { fprintf(stdout, "al_destroy_event_queue\n"); }
  }

  memset(&pd->al_ctx, 0, sizeof(struct EZALAllegroContext));
  memset(&pd->rt_ctx, 0, sizeof(struct EZALRuntimeContext));
  memset(pd, 0, sizeof(struct EZALPrivateData));

  return true;
}

// main loop
bool ezal_private_run(struct EZALPrivateData* pd)
{
  pd->rt_ctx.is_running = true;
  pd->rt_ctx.should_redraw = false;
  pd->rt_ctx.create(&pd->rt_ctx);

  if (pd->cfg.debug) { fprintf(stdout, "starting main loop\n"); }
  ezal_private_resize(pd);
  al_start_timer(pd->al_ctx.timer);
  while (pd->rt_ctx.is_running)
  {
    pd->update(pd);
    if (pd->rt_ctx.should_redraw && al_is_event_queue_empty(pd->al_ctx.event_queue))
    {
      pd->rt_ctx.should_redraw = false;
      pd->render(pd);
      pd->rt_ctx.render(&pd->rt_ctx);
      pd->present(pd);
    }
  }
  if (pd->cfg.debug) { fprintf(stdout, "main loop finished\n"); }

  pd->rt_ctx.destroy(&pd->rt_ctx);

  return true;
}

void ezal_private_dump_configuration_to_stdout(struct EZALPrivateData* pd)
{
  #define EZALYESNO(x) (x?"YES":"NO")
  const char* fmt = "configuration:\n"
    "  width = %d\n"
    "  height = %d\n"
    "  logical width = %d\n"
    "  logical height = %d\n"
    "  audio samples = %d\n"
    "  frame rate = %d\n"
    "  fullscreen = %s\n"
    "  auto scaling = %s\n"
    "  stretch scaling = %s\n"
    "  audio enabled = %s\n"
    "  mouse enabled = %s\n"
    "  keyboard enabled = %s\n"
    "  debug = %s\n\n";

  fprintf(
    stdout,
    fmt,
    pd->cfg.width,
    pd->cfg.height,
    pd->cfg.logical_width,
    pd->cfg.logical_height,
    pd->cfg.audio_samples,
    pd->cfg.frame_rate,
    EZALYESNO(pd->cfg.fullscreen),
    EZALYESNO(pd->cfg.auto_scale),
    EZALYESNO(pd->cfg.stretch_scale),
    EZALYESNO(pd->cfg.enable_audio),
    EZALYESNO(pd->cfg.enable_mouse),
    EZALYESNO(pd->cfg.enable_keyboard),
    EZALYESNO(pd->cfg.debug));
  #undef EZALYESNO
}

int ezal_private_adapter_start(struct EZALRuntimeAdapter* rta)
{
  if (!rta)
  {
    fprintf(stderr, "Fatal Error: rta is not a valid EZALRuntimeAdapter pointer\n");
    exit(EXIT_FAILURE);
  }

  struct EZALPrivateData* pd = (struct EZALPrivateData*)rta->rt_ctx->_ezal_reserved;

  if (!ezal_private_run(pd))
  {
    exit(EXIT_FAILURE);
  }

  if (!ezal_private_quit(pd))
  {
    exit(EXIT_FAILURE);
  }

  free(pd);
  free(rta);

  return EXIT_SUCCESS;
}

// public api functions

void ezal_use_config_defaults(struct EZALConfig* cfg)
{
  if (!cfg)
  {
    fprintf(stderr, "Error: ezal_use_config_defaults requires a valid EZALConfig pointer\n");
    return;
  }

  cfg->width = 800;
  cfg->height = 600;
  cfg->logical_width = 800;
  cfg->logical_height = 600;
  cfg->fullscreen = false;
  cfg->auto_scale = false;
  cfg->stretch_scale = false;
  cfg->audio_samples = 1;
  cfg->enable_audio = true;
  cfg->enable_mouse = true;
  cfg->enable_keyboard = true;
  cfg->debug = false;
  cfg->frame_rate = 30;
}

/**
 * @brief ezal program entry point
 * The main game loop and calls to the user functions are all
 * handled behind the scenes with this one function call.
 * @param title name of your game
 * @param create user function for creation (before main loop)
 * @param destroy user function for destruction (after main loop)
 * @param update user function for update (in main loop every frame)
 * @param render user function for render (in main loop every frame)
 * @param cfg optional configuration use zero for default
 * @return int returns 0 on success and not zero on failure
 */
int ezal_start(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg
)
{
  struct EZALPrivateData pd_obj;
  struct EZALPrivateData* pd = &pd_obj;

  ezal_private_copy_config(cfg, &pd->cfg);
  ezal_private_dump_configuration_to_stdout(pd);

  if (!ezal_private_init(pd, create, destroy, update, render))
  {
    exit(EXIT_FAILURE);
  }

  if (pd->cfg.debug) { fprintf(stdout, "starting %s\n", title); }

  al_set_window_title(pd->al_ctx.display, title);

  if (!ezal_private_run(pd))
  {
    exit(EXIT_FAILURE);
  }

  if (!ezal_private_quit(pd))
  {
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}

struct EZALRuntimeAdapter* ezal_init(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg)
{
  struct EZALPrivateData* pd = (struct EZALPrivateData*)malloc(sizeof(struct EZALPrivateData));

  ezal_private_copy_config(cfg, &pd->cfg);
  ezal_private_dump_configuration_to_stdout(pd);

  if (!ezal_private_init(pd, create, destroy, update, render))
  {
    exit(EXIT_FAILURE);
  }

  al_set_window_title(pd->al_ctx.display, title);

  struct EZALRuntimeAdapter* rta = (struct EZALRuntimeAdapter*)malloc(sizeof(struct EZALRuntimeAdapter));

  rta->rt_ctx = &pd->rt_ctx;
  rta->rt_ctx->_ezal_reserved = pd;
  rta->start = &ezal_private_adapter_start;

  return rta;
}

void ezal_stop(struct EZALRuntimeContext* ctx)
{
  if (!ctx)
  {
    return;
  }
  ctx->is_running = false;
  ctx->should_redraw = false;
}
