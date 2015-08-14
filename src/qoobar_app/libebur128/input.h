/* See LICENSE file for copyright and license details. */
#ifndef _INPUT_H_
#define _INPUT_H_

#ifdef WITH_DECODING

#include <string.h>

struct input_handle;

struct input_ops {
  unsigned      (*get_channels)(struct input_handle* ih);
  unsigned long (*get_samplerate)(struct input_handle* ih);
  float*        (*get_buffer)(struct input_handle* ih);
  struct input_handle* (*handle_init)();
  void          (*handle_destroy)(struct input_handle** ih);
  int           (*open_file)(struct input_handle* ih, const char* filename);
  int           (*set_channel_map)(struct input_handle* ih, int* st);
  int           (*allocate_buffer)(struct input_handle* ih);
  size_t        (*get_total_frames)(struct input_handle* ih);
  size_t        (*read_frames)(struct input_handle* ih);
  void          (*free_buffer)(struct input_handle* ih);
  void          (*close_file)(struct input_handle* ih);
  int           (*init_library)();
  void          (*exit_library)();
};

enum {PLUGIN_GSTREAMER, PLUGIN_FFMPEG};

int input_init(int forced_plugin);
int input_deinit(void);
struct input_ops* input_get_ops();

#endif

#endif  /* _INPUT_H_ */
