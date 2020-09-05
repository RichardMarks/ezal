# EZAL API

## Important Data Structures

```c
struct EZALConfig;
```
This structure contains optional configuration variables to control the initialization of the library and runtime.

+ `int width;` - window width in pixels
+ `int height;` - window height in pixels
+ `int logical_width;` - game width in pixels
+ `int logical_height;` - game height in pixels
+ `int audio_samples;` - max number of concurrent audio samples
+ `int frame_rate;` - number of frames per second for main loop
+ `bool fullscreen;` - fill the screen (*true*) or run in a window (*false*)
+ `bool auto_scale;` - scale game size to window size
+ `bool stretch_scale;` - stretch game to window (*true*) or fit (*false*)
+ `bool enable_audio;` - you want to have sound capabilities?
+ `bool enable_mouse;` - you want to have mouse capabilities?
+ `bool enable_keyboard;` - you want to have keyboard capabilities?
+ `bool debug;` - you want to get details on `stdout` during runtime

```c
struct EZALAllegroContext
```
This structure wraps all the Allegro variables and input tracking variables used by the runtime.

+ `ALLEGRO_TIMER* timer;`
+ `ALLEGRO_DISPLAY* display;`
+ `ALLEGRO_BITMAP* buffer;`
+ `ALLEGRO_FONT* font;`
+ `ALLEGRO_EVENT_QUEUE* event_queue;`
+ `ALLEGRO_EVENT event;`
+ `unsigned char key[ALLEGRO_KEY_MAX];`
+ `float mouseX;`
+ `float mouseY;`
+ `unsigned int mouseButton;`
+ `unsigned char mouseState;`

```c
struct EZALRuntimeContext
```
This structure holds pointers to the other data structures and some other data the runtime uses to operate.

+ `struct EZALConfig* cfg;` - pointer to configuration data
+ `struct EZALAllegroContext* al_ctx;` - pointer to Allegro data
+ `void* user[EZAL_MAX_USER_DATA_PTRS];` - array of pointers to user data
> \*\* There are other fields that you do not usually need to access.

## Public API Functions

Fill the cfg data structure with the default values with the `ezal_use_config_defaults` function.
```c
void ezal_use_config_defaults(struct EZALConfig* cfg);
```

Start your game with the `ezal_start` function. Pass the name of your game as a C-string and pointers to your `EZAL_FN` functions.

```c
int ezal_start(
  const char* title,
  EZALFPTR create,
  EZALFPTR destroy,
  EZALFPTR update,
  EZALFPTR render,
  struct EZALConfig* cfg
);
```

To stop the runtime and exit the program, call the `ezal_stop` function and pass the `ctx` from the `EZAL_FN` function.

```c
void ezal_stop(struct EZALRuntimeContext* ctx);
```

## C Macros
There are a few macros that make your code a little bit *cleaner*.

The first of these macros is the `EZAL_FN` macro which helps you declare the functions that `ezal_start` requires.
```c
#define EZAL_FN(identifier)
```

For example,
```c
EZAL_FN(my_create_fn)
{
  // there is a "hidden" param called "ctx" that is
  // available to you in this scope. It is a pointer to
  // an EZALRuntimeContext struct
  // (struct EZALRuntimeContext* ctx)
  // the return type of the EZAL_FN function is void
}
```

You do **not** have to use the `EZAL_FN` macro, you could write your function declarations like this:

```c
void my_create_fn(struct EZALRuntimeContext* ctx)
{

}
```
> You should use the macro in case EZAL is updated in the future and the function signatures change, your code would not break if you are using the macro.

The next macro is to help your input handling code be a little bit cleaner.
```c
#define EZAL_KEY(keycode)
```

You would use this macro within an `EZAL_FN` function body in order to test for keyboard input (when `cfg.enable_keyboard = true`) like this:

```c
if (EZAL_KEY(ALLEGRO_KEY_ESCAPE))
{
  ezal_stop(ctx);
}
```

You do not *have* to use the `EZAL_KEY` macro, and you could write the if statement could be written like this:

```c
if (ctx->al_ctx->key[ALLEGRO_KEY_ESCAPE])
{
  ezal_stop(ctx);
}
```
> But you should probably use the macro, because it is nicer to read

The last macro or `pre-processor definition` to be more exact is a one-time-use before you `#include <ezal.h>` in order to specify the number of `user-data` pointers you want to have.
```c
#define EZAL_MAX_USER_DATA_PTRS { an_integer >= 1 }
```

By default, you only get `1 user-data pointer` in the `EZALRuntimeContext` structure.

**WHAT IS A USER-DATA POINTER?** (*I hear you screaming from here*)

A `user-data` pointer is a `void pointer` that can point to anything you want.

### TODO: An example of using user-data is needed
