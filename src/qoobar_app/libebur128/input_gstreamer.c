/* See LICENSE file for copyright and license details. */
#ifdef WITH_DECODING
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/audio/audio-channels.h>

#include <locale.h>
#include <stdio.h>
#include <stddef.h>
#include "input.h"
#include "ebur128.h"

struct input_handle {
  const char *filename;
  GThread *gstreamer_loop;
  gboolean quit_pipeline;
  gboolean ready;
  gboolean main_loop_quit;

  GMainContext *main_context;
  GSource *message_source;

  GstElement *bin;
  GstElement *appsink;
  GMainLoop *loop;

  GSList *buffer_list;
  size_t current_bytes;

  float *buffer;

  gint n_channels;
  gint sample_rate;
  GstAudioChannelPosition *channel_positions;
};

static GMutex gstreamer_mutex;
static gboolean verbose = FALSE;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  struct input_handle *ih = (struct input_handle *) data;

  /* if (verbose) fprintf(stderr, "%p %s %s\n", bus, GST_MESSAGE_TYPE_NAME(msg), ih->filename); */

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ASYNC_DONE:{
      ih->quit_pipeline = FALSE;
      ih->ready = TRUE;
      break;
    }
    case GST_MESSAGE_EOS:{
      if (verbose) g_print("End-of-stream\n");
      ih->ready = TRUE;
      g_mutex_lock(&gstreamer_mutex);
      if (!ih->main_loop_quit) {
        ih->main_loop_quit = TRUE;
        g_main_loop_quit(ih->loop);
      }
      g_mutex_unlock(&gstreamer_mutex);
      break;
    }
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *err;

      gst_message_parse_error (msg, &err, &debug);
      g_free (debug);

      if (verbose) g_print("%p Error: %s\n", bus, err->message);
      g_error_free (err);

      ih->ready = TRUE;
      g_mutex_lock(&gstreamer_mutex);
      if (!ih->main_loop_quit) {
        ih->main_loop_quit = TRUE;
        g_main_loop_quit(ih->loop);
      }
      g_mutex_unlock(&gstreamer_mutex);
      break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state;

      gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
      /* if (verbose) g_print ("Element %s changed state from %s to %s.\n",
             GST_OBJECT_NAME (msg->src),
             gst_element_state_get_name (old_state),
             gst_element_state_get_name (new_state)); */
      break;
    }
    case GST_MESSAGE_STREAM_STATUS:{
      GstStreamStatusType type;
      GstElement *owner;
      gst_message_parse_stream_status(msg, &type, &owner);
      /* if (verbose) g_print("%p New Stream Type: %d\n", bus, type); */
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static gboolean query_data(struct input_handle* ih) {
  //GstBuffer *preroll;
  GstCaps *src_caps;
  GstStructure *s;
  GstSample *sample;
  int i;

  ih->n_channels = 0;
  ih->sample_rate = 0;
  ih->channel_positions = NULL;

  sample =  gst_app_sink_pull_preroll(GST_APP_SINK_CAST(ih->appsink));
  src_caps = gst_sample_get_caps(sample);
  //preroll = GST_BUFFER(gst_app_sink_pull_preroll(GST_APP_SINK(ih->appsink)));
  //src_caps = gst_buffer_get_caps(preroll);

  s = gst_caps_get_structure(src_caps, 0);
  gst_structure_get_int(s, "rate", &(ih->sample_rate));
  gst_structure_get_int(s, "channels", &(ih->n_channels));
  //preroll = gst_sample_get_buffer(sample);
  if (!ih->sample_rate || !ih->n_channels) {
    gst_caps_unref(src_caps);
    gst_sample_unref(sample);
    //gst_buffer_unref(preroll);
    return FALSE;
  }

  GstAudioInfo info;
  if (gst_audio_info_from_caps(&info, src_caps)) {
    ih->channel_positions = g_new (GstAudioChannelPosition, ih->n_channels);
    memcpy (ih->channel_positions, info.position,
          info.channels * sizeof (info.position[0]));
  }

  if (verbose) {
    if (ih->channel_positions) {
      for (i = 0; i < ih->n_channels; ++i) {
        printf("Channel %d: %d\n", i, ih->channel_positions[i]);
      }
    }
    g_print ("%d channels @ %d Hz\n", ih->n_channels, ih->sample_rate);
  }

  gst_caps_unref(src_caps);
  gst_sample_unref(sample);
  //gst_buffer_unref(preroll);

  return TRUE;
}

unsigned gstreamer_get_channels(struct input_handle* ih) {
  return (unsigned) ih->n_channels;
}

unsigned long gstreamer_get_samplerate(struct input_handle* ih) {
  return (unsigned long) ih->sample_rate;
}

float* gstreamer_get_buffer(struct input_handle* ih) {
  return ih->buffer;
}

struct input_handle* gstreamer_handle_init() {
  struct input_handle* ret;
  ret = malloc(sizeof(struct input_handle));

  ret->buffer_list = NULL;
  ret->current_bytes = 0;

  ret->main_context = g_main_context_new();
  ret->channel_positions = NULL;

  return ret;
}

static gpointer gstreamer_loop(struct input_handle *ih) {
  GstElement *fdsrc;
  GError *error = NULL;
  GstBus *bus;

  ih->bin = gst_parse_launch("filesrc name=my_fdsrc ! "
                             "decodebin2 ! "
                             "audioconvert name=converter ! "
#if 1
                             "audio/x-raw-float,width=32,endianness=1234 ! "
                             "appsink name=sink sync=FALSE", &error);
#else
                             "audio/x-raw-float,width=32,endianness=1234 ! "
                             "ffenc_ac3 bitrate=128000 ! ffmux_ac3 ! filesink location=tmp.ac3", &error);
#endif
  if (!ih->bin) {
    fprintf(stderr, "Parse error: %s", error->message);
    return NULL;
  }

  fdsrc = gst_bin_get_by_name(GST_BIN(ih->bin), "my_fdsrc");
  g_object_set(G_OBJECT(fdsrc), "location", ih->filename, NULL);

  ih->appsink = gst_bin_get_by_name(GST_BIN(ih->bin), "sink");
  /* gst_app_sink_set_max_buffers(GST_APP_SINK(ih->appsink), 1); */

  /* start playing */
  ih->loop = g_main_loop_new(ih->main_context, FALSE);

  bus = gst_element_get_bus(ih->bin);
  ih->message_source = gst_bus_create_watch(bus);
  g_source_set_callback(ih->message_source, (GSourceFunc) bus_call, ih, NULL);
  g_source_attach(ih->message_source, ih->main_context);
  g_source_unref(ih->message_source);
  g_object_unref(bus);

  /* start play back and listed to events */
  gst_element_set_state(ih->bin, GST_STATE_PLAYING);

  g_main_loop_run(ih->loop);

  return NULL;
}

int gstreamer_open_file(struct input_handle* ih, const char* filename) {
  gint64 beg, end;

  ih->filename = filename;
  ih->quit_pipeline = TRUE;
  ih->main_loop_quit = FALSE;
  ih->ready = FALSE;
  ih->bin = NULL;
  ih->message_source = NULL;
  ih->gstreamer_loop = g_thread_new("thr",(GThreadFunc) gstreamer_loop, ih);

  beg = g_get_real_time();
  while (!ih->ready) {
    g_thread_yield();
    end = g_get_real_time();
    if (end - beg > 1 * G_USEC_PER_SEC) {
      break;
    }
  }

  if (!ih->quit_pipeline) {
    if (!query_data(ih)) {
      ih->quit_pipeline = TRUE;
    }
  }

  if (ih->quit_pipeline) {
    if (ih->bin) {
      GstBus *bus = gst_element_get_bus(ih->bin);
      gst_bus_post(bus, gst_message_new_eos(NULL));
      g_object_unref(bus);
    }
    g_thread_join(ih->gstreamer_loop);
    if (ih->message_source) g_source_destroy(ih->message_source);
    if (ih->bin) {
      /* cleanup */
      gst_element_set_state(ih->bin, GST_STATE_NULL);
      g_object_unref(ih->bin);
      ih->bin = NULL;
      g_main_loop_unref(ih->loop);
    }
    return 1;
  } else {
    return 0;
  }
}

int gstreamer_set_channel_map(struct input_handle* ih, int* st) {
  gint j;
  for (j = 0; j < ih->n_channels; ++j) {
    switch (ih->channel_positions[j]) {
      case GST_AUDIO_CHANNEL_POSITION_INVALID:
        st[j] = EBUR128_UNUSED;         break;
      case GST_AUDIO_CHANNEL_POSITION_MONO:
        st[j] = EBUR128_CENTER;         break;
      case GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT:
        st[j] = EBUR128_LEFT;           break;
      case GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT:
        st[j] = EBUR128_RIGHT;          break;
      case GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER:
        st[j] = EBUR128_CENTER;         break;
      case GST_AUDIO_CHANNEL_POSITION_REAR_LEFT:
        st[j] = EBUR128_LEFT_SURROUND;  break;
      case GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT:
        st[j] = EBUR128_RIGHT_SURROUND; break;
      default:
        st[j] = EBUR128_UNUSED;         break;
    }
  }
  return 0;
}

void gstreamer_handle_destroy(struct input_handle** ih) {
  g_free((*ih)->channel_positions);
  g_main_context_unref((*ih)->main_context);
  free(*ih);
  *ih = NULL;
}

#define BUFFER_SIZE ((size_t) ih->sample_rate * sizeof(float))
int gstreamer_allocate_buffer(struct input_handle* ih) {
  ih->buffer = g_malloc(BUFFER_SIZE);
  return 0;
}

size_t gstreamer_get_total_frames(struct input_handle* ih) {
  gint64 time = 0;
  GstFormat format = GST_FORMAT_TIME;

  if (gst_element_query_duration(ih->bin, format, &time)) {
      double tmp = time * 1e-9 * ih->sample_rate;
      if (tmp <= 0.0) {
          return 0;
      } else {
          return (size_t) (tmp + 0.5);
      }
  } else {
      return 0;
  }
}

size_t gstreamer_read_frames(struct input_handle* ih) {
    size_t buf_pos = 0;
    GSList *next;

    while (ih->current_bytes < BUFFER_SIZE) {
        GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK_CAST(ih->appsink));
        if (!sample) break;
        GstBuffer *buf = gst_sample_get_buffer(sample);
        //GstBuffer *buf = gst_app_sink_pull_buffer(GST_APP_SINK_CAST(ih->appsink));
        if (!buf) {
            gst_sample_unref(sample);
            break;
        }
        ih->buffer_list = g_slist_append(ih->buffer_list, sample);
        ih->current_bytes += gst_buffer_get_size(buf);
        gst_buffer_unref(buf);
        //ih->current_bytes += buf->size;
    }

    while (ih->buffer_list) {
        GstSample *sample = GST_SAMPLE_CAST(ih->buffer_list->data);
        GstBuffer *buf = gst_sample_get_buffer(sample);
        gsize s = gst_buffer_get_size(buf);
        if (s + buf_pos <= BUFFER_SIZE) {
            gst_buffer_unref(buf);
            gst_sample_unref(sample);
            break;
        }
        GstMapInfo info;
        gst_buffer_map(buf, &info, GST_MAP_READ);
        memcpy((guint8 *) ih->buffer + buf_pos,  info.data, s);
        buf_pos           += s;
        ih->current_bytes -= s;
        gst_buffer_unmap(buf, &info);

        gst_buffer_unref(buf);
        gst_sample_unref(sample);
        next = ih->buffer_list->next;
        g_slist_free_1(ih->buffer_list);
        ih->buffer_list = next;
    }

    return buf_pos / sizeof(float) / gstreamer_get_channels(ih);
}

void gstreamer_free_buffer(struct input_handle* ih) {
  g_free(ih->buffer);
  return;
}

void gstreamer_close_file(struct input_handle* ih) {
  if (ih->bin) {
    GstBus *bus = gst_element_get_bus(ih->bin);
    gst_bus_post(bus, gst_message_new_eos(NULL));
    g_object_unref(bus);
  }
  g_thread_join(ih->gstreamer_loop);
  if (ih->message_source) g_source_destroy(ih->message_source);
  if (ih->bin) {
    /* cleanup */
    gst_element_set_state(ih->bin, GST_STATE_NULL);
    g_object_unref(ih->bin);
    ih->bin = NULL;
    g_main_loop_unref(ih->loop);
  }
}

int gstreamer_init_library() {
  char *current_locale = setlocale(LC_ALL, NULL);
  gst_init(NULL, NULL);
  setlocale(LC_ALL, current_locale);
  return 0;
}

void gstreamer_exit_library() {
  gst_deinit();
  return;
}
#endif
