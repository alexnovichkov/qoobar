#ifndef INPUT_FFMPEG_H
#define INPUT_FFMPEG_H

#ifdef WITH_DECODING
#ifdef USE_FFMPEG

unsigned ffmpeg_get_channels(struct input_handle* ih);
unsigned long ffmpeg_get_samplerate(struct input_handle* ih);
float* ffmpeg_get_buffer(struct input_handle* ih);
struct input_handle* ffmpeg_handle_init();
void ffmpeg_handle_destroy(struct input_handle** ih);
int ffmpeg_open_file(struct input_handle* ih, const char* filename);
int ffmpeg_set_channel_map(struct input_handle* ih, int* st);
int ffmpeg_allocate_buffer(struct input_handle* ih);
size_t ffmpeg_get_total_frames(struct input_handle* ih);
size_t ffmpeg_read_frames(struct input_handle* ih);
void ffmpeg_free_buffer(struct input_handle* ih);
void ffmpeg_close_file(struct input_handle* ih);
int ffmpeg_init_library();
void ffmpeg_exit_library();


struct input_ops ffmpeg_ip_ops = {
  ffmpeg_get_channels,
  ffmpeg_get_samplerate,
  ffmpeg_get_buffer,
  ffmpeg_handle_init,
  ffmpeg_handle_destroy,
  ffmpeg_open_file,
  ffmpeg_set_channel_map,
  ffmpeg_allocate_buffer,
  ffmpeg_get_total_frames,
  ffmpeg_read_frames,
  ffmpeg_free_buffer,
  ffmpeg_close_file,
  ffmpeg_init_library,
  ffmpeg_exit_library
};

#endif
#endif

#endif // INPUT_FFMPEG_H
