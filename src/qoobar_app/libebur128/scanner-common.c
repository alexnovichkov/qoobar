#include "scanner-common.h"

#ifdef WITH_DECODING
#include <stdlib.h>
#include <stdio.h>
#include <glib/gstdio.h>

extern gboolean verbose;

static struct file_data empty;

int open_plugin(const char *raw, const char *display,
                struct input_ops **ops,
                struct input_handle **ih)
{
    int result;

    *ops = input_get_ops();
    if (!(*ops)) {
        if (verbose) fprintf(stderr, "No plugin found for file '%s'\n", display);
        return 1;
    }
    *ih = (*ops)->handle_init();

    result = (*ops)->open_file(*ih, raw);
    if (result) {
        if (verbose) fprintf(stderr, "Error opening file '%s'\n", display);
        return 1;
    }
    return 0;
}

void init_and_get_number_of_frames(struct filename_list_node *fln, int *do_scan)
{
    struct file_data *fd;

    struct input_ops *ops = NULL;
    struct input_handle *ih = NULL;
    int result;

    fln->d = g_malloc(sizeof(struct file_data));
    memcpy(fln->d, &empty, sizeof empty);
    fd = (struct file_data *) fln->d;

    result = open_plugin(fln->fr->raw, fln->fr->display, &ops, &ih);
    if (result) {
        goto free;
    }

    *do_scan = TRUE;
    fd->number_of_frames = ops->get_total_frames(ih);
  free:
    if (!result) ops->close_file(ih);
    if (ih) ops->handle_destroy(&ih);
}

void init_state_and_scan_work_item(struct filename_list_node *fln)
{
    struct file_data *fd = (struct file_data *) fln->d;

    struct input_ops* ops = NULL;
    struct input_handle* ih = NULL;
    int r128_mode = EBUR128_MODE_I;
    unsigned int i;
    int *channel_map;

    int result;
    float *buffer = NULL;
    size_t nr_frames_read;

    result = open_plugin(fln->fr->raw, fln->fr->display, &ops, &ih);
    if (result) {
        goto free;
    }

    r128_mode |= EBUR128_MODE_SAMPLE_PEAK;

    fd->st = ebur128_init(ops->get_channels(ih),
                          ops->get_samplerate(ih),
                          r128_mode);

    channel_map = g_malloc(fd->st->channels * sizeof(int));
    if (!ops->set_channel_map(ih, channel_map)) {
        for (i = 0; i < fd->st->channels; ++i) {
            ebur128_set_channel(fd->st, i, channel_map[i]);
        }
    }
    free(channel_map);

    if (fd->st->channels == 1) {
        ebur128_set_channel(fd->st, 0, EBUR128_DUAL_MONO);
    }

    result = ops->allocate_buffer(ih);
    if (result) abort();
    buffer = ops->get_buffer(ih);

    while ((nr_frames_read = ops->read_frames(ih))) {
        fd->number_of_elapsed_frames += nr_frames_read;
        result = ebur128_add_frames_float(fd->st, buffer, nr_frames_read);
        if (result) abort();
    }


    if (fd->number_of_elapsed_frames != fd->number_of_frames) {
        if (verbose) {
            fprintf(stderr,
                    "Warning: Could not read full file or determine right length: "
                    "Expected: %lu Got: %lu",
                    (long unsigned int)fd->number_of_frames,
                    (long unsigned int)fd->number_of_elapsed_frames);
        }
    }
    ebur128_loudness_global(fd->st, &fd->loudness);

    if ((fd->st->mode & EBUR128_MODE_SAMPLE_PEAK) == EBUR128_MODE_SAMPLE_PEAK) {
        for (i = 0; i < fd->st->channels; ++i) {
            double sp;
            ebur128_sample_peak(fd->st, i, &sp);
            if (sp > fd->peak) {
                fd->peak = sp;
            }
        }
    }
    fd->scanned = TRUE;

    if (ih) ops->free_buffer(ih);
  free:
    if (!result) ops->close_file(ih);
    if (ih) ops->handle_destroy(&ih);
}

void init_state_and_scan(gpointer work_item, GThreadPool *pool)
{
    g_thread_pool_push(pool, work_item, NULL);
}

void destroy_state(struct filename_list_node *fln, gpointer unused)
{
    struct file_data *fd = (struct file_data *) fln->d;

    (void) unused;
    if (fd->st) {
        ebur128_destroy(&fd->st);
    }
}

void get_state(struct filename_list_node *fln, GPtrArray *states)
{
    struct file_data *fd = (struct file_data *) fln->d;

    if (fd->scanned) {
       g_ptr_array_add(states, fd->st);
    }
}

void get_max_peaks(struct filename_list_node *fln, struct file_data *result)
{
    struct file_data *fd = (struct file_data *) fln->d;

    if (fd->scanned) {
        if (fd->peak > result->peak) result->peak = fd->peak;
    }
}

void process_files(GSList *files)
{
    g_slist_foreach(files, (GFunc) init_state_and_scan_work_item, NULL);

}
#endif
