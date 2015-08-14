#ifndef SCANNER_TAG_H
#define SCANNER_TAG_H

#ifdef WITH_DECODING
#include <glib.h>

#include "filetree.h"

#define RG_REFERENCE_LEVEL -18.0
double clamp_rg(double x);

void tag_file(struct filename_list_node *fln, int *ret);
//int scan_files(GSList *files);
//int tag_files(GSList *files);
void calculate_album_gain_and_peak_last_dir(void);
void calculate_album_gain_and_peak(struct filename_list_node *fln, gpointer unused);
//int loudness_tag(GSList *files);
//gboolean loudness_tag_parse(int *argc, char **argv[]);

#endif
#endif /* end of include guard: SCANNER_TAG_H */
