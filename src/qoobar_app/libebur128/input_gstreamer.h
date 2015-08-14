#ifndef INPUT_GSTREAMER_H
#define INPUT_GSTREAMER_H

#ifdef WITH_DECODING

int gstreamer_init_library();
void gstreamer_exit_library();
void gstreamer_free_buffer(struct input_handle* ih);
void gstreamer_close_file(struct input_handle* ih);
size_t gstreamer_get_total_frames(struct input_handle* ih);
size_t gstreamer_read_frames(struct input_handle* ih);
int gstreamer_allocate_buffer(struct input_handle* ih);
void gstreamer_handle_destroy(struct input_handle** ih);
int gstreamer_set_channel_map(struct input_handle* ih, int* st);
int gstreamer_open_file(struct input_handle* ih, const char* filename);
unsigned gstreamer_get_channels(struct input_handle* ih);
unsigned long gstreamer_get_samplerate(struct input_handle* ih);
float* gstreamer_get_buffer(struct input_handle* ih);
struct input_handle* gstreamer_handle_init();


struct input_ops gstreamer_ip_ops =
{
  gstreamer_get_channels,
  gstreamer_get_samplerate,
  gstreamer_get_buffer,
  gstreamer_handle_init,
  gstreamer_handle_destroy,
  gstreamer_open_file,
  gstreamer_set_channel_map,
  gstreamer_allocate_buffer,
  gstreamer_get_total_frames,
  gstreamer_read_frames,
  gstreamer_free_buffer,
  gstreamer_close_file,
  gstreamer_init_library,
  gstreamer_exit_library
};
#endif

#endif // INPUT_GSTREAMER_H
