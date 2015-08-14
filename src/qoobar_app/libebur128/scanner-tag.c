#include "scanner-tag.h"

#ifdef WITH_DECODING
#include <stdlib.h>
#include <stdio.h>

#include "scanner-common.h"

static struct file_data empty;

double clamp_rg(double x)
{
    if (x < -51.0) return -51.0;
    else if (x > 51.0) return 51.0;
    else return x;
}

static void fill_album_data(struct filename_list_node *fln, double *album_data)
{
    struct file_data *fd = (struct file_data *) fln->d;

    fd->gain_album = album_data[0];
    fd->peak_album = album_data[1];
}

static gchar *current_dir;
static GSList *files_in_current_dir;

void calculate_album_gain_and_peak_last_dir(void)
{
    double album_data[] = {0.0, 0.0};
    GPtrArray *states = g_ptr_array_new();
    struct file_data result;
    memcpy(&result, &empty, sizeof empty);

    files_in_current_dir = g_slist_reverse(files_in_current_dir);
    g_slist_foreach(files_in_current_dir, (GFunc) get_state, states);
    ebur128_loudness_global_multiple((ebur128_state **) states->pdata,
                                     states->len, &album_data[0]);
    album_data[0] = clamp_rg(RG_REFERENCE_LEVEL - album_data[0]);
    g_slist_foreach(files_in_current_dir, (GFunc) get_max_peaks, &result);
    album_data[1] = result.peak;
    g_slist_foreach(files_in_current_dir, (GFunc) fill_album_data, album_data);

    g_ptr_array_free(states, TRUE);

    g_free(current_dir);
    current_dir = NULL;
    g_slist_free(files_in_current_dir);
    files_in_current_dir = NULL;
}

void calculate_album_gain_and_peak(struct filename_list_node *fln, gpointer unused)
{
    gchar *dirname;

    (void) unused;
    dirname = g_path_get_dirname(fln->fr->raw);
    if (!current_dir) {
        current_dir = g_strdup(dirname);
    }
    if (!strcmp(current_dir, dirname)) {
        files_in_current_dir = g_slist_prepend(files_in_current_dir, fln);
    } else {
        calculate_album_gain_and_peak_last_dir();
        current_dir = g_strdup(dirname);
        files_in_current_dir = g_slist_prepend(files_in_current_dir, fln);
    }
    g_free(dirname);
}


#endif
