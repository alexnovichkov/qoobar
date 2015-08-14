#ifndef FILETREE_H
#define FILETREE_H

#ifdef WITH_DECODING

#include <glib.h>

typedef void *Filetree;

#define FILETREE_DIR 1
#define FILETREE_FILE 2

/* raw: glib filename encoding
 * display: UTF-8 encoded string
 * collate_key: for sorting
 * type: either FILETREE_DIR or FILETREE_FILE */
struct filename_representations {
    char *raw;
    char *display;
    char *collate_key;
    int type;
    int _; /* padding */
};

struct filename_list_node {
    struct filename_representations *fr;
    void *d;
};

Filetree filetree_init(char *roots[],
                       size_t roots_size,
                       gboolean recursive,
                       gboolean follow_symlinks,
                       gboolean no_sort,
                       GSList **errors);
void filetree_print(Filetree tree);
void filetree_destroy(Filetree tree);
void filetree_file_list(Filetree tree, GSList **files);

/* traversal functions */
void filetree_print_error(gpointer user, gpointer user_data);
void filetree_free_error(gpointer user, gpointer user_data);
void filetree_get_file_size(gpointer user, gpointer user_data);
void filetree_print_file_size(gpointer user, gpointer user_data);

void filetree_free_list_entry(gpointer user, gpointer user_data);
void filetree_remove_common_prefix(GSList *files);

#endif

#endif /* end of include guard: FILETREE_H */

