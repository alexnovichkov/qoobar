#include "filetree.h"

#ifdef WITH_DECODING

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gstdio.h>

static const unsigned char strescape_exceptions[] = {
    0x7f,  0x80,  0x81,  0x82,  0x83,  0x84,  0x85,  0x86,
    0x87,  0x88,  0x89,  0x8a,  0x8b,  0x8c,  0x8d,  0x8e,
    0x8f,  0x90,  0x91,  0x92,  0x93,  0x94,  0x95,  0x96,
    0x97,  0x98,  0x99,  0x9a,  0x9b,  0x9c,  0x9d,  0x9e,
    0x9f,  0xa0,  0xa1,  0xa2,  0xa3,  0xa4,  0xa5,  0xa6,
    0xa7,  0xa8,  0xa9,  0xaa,  0xab,  0xac,  0xad,  0xae,
    0xaf,  0xb0,  0xb1,  0xb2,  0xb3,  0xb4,  0xb5,  0xb6,
    0xb7,  0xb8,  0xb9,  0xba,  0xbb,  0xbc,  0xbd,  0xbe,
    0xbf,  0xc0,  0xc1,  0xc2,  0xc3,  0xc4,  0xc5,  0xc6,
    0xc7,  0xc8,  0xc9,  0xca,  0xcb,  0xcc,  0xcd,  0xce,
    0xcf,  0xd0,  0xd1,  0xd2,  0xd3,  0xd4,  0xd5,  0xd6,
    0xd7,  0xd8,  0xd9,  0xda,  0xdb,  0xdc,  0xdd,  0xde,
    0xdf,  0xe0,  0xe1,  0xe2,  0xe3,  0xe4,  0xe5,  0xe6,
    0xe7,  0xe8,  0xe9,  0xea,  0xeb,  0xec,  0xed,  0xee,
    0xef,  0xf0,  0xf1,  0xf2,  0xf3,  0xf4,  0xf5,  0xf6,
    0xf7,  0xf8,  0xf9,  0xfa,  0xfb,  0xfc,  0xfd,  0xfe,
    0xff,
    '\0'
};

static int compare_filenames(gconstpointer lhs,
                             gconstpointer rhs,
                             gpointer user_data) {
    struct filename_representations const *fr_l = lhs;
    struct filename_representations const *fr_r = rhs;
    (void) user_data;
    if (fr_l->type != fr_r->type) {
        return fr_l->type - fr_r->type;
    }
    return strcmp(fr_l->collate_key, fr_r->collate_key);
}

static int compare_insert_last(gconstpointer lhs,
                               gconstpointer rhs,
                               gpointer user_data) {
    (void) lhs;
    (void) rhs;
    (void) user_data;
    return 1;
}

static struct filename_representations
*filename_representations_new(const char *raw, int type)
{
    struct filename_representations *fr;
    char *display_pre;
    char *real_path;

    fr = g_malloc(sizeof(struct filename_representations));
    fr->type = type;
    fr->raw = g_strdup(raw);

    real_path = realpath(fr->raw, NULL);
    display_pre = g_filename_display_name(real_path);
    fr->collate_key = g_utf8_collate_key_for_filename(display_pre, -1);

    fr->display = g_strescape(display_pre,
                              (const gchar *) strescape_exceptions);
    g_free(display_pre);
    g_free(real_path);
    return fr;
}

static void filename_representations_free(void *fr)
{
    g_free(((struct filename_representations *) fr)->raw);
    g_free(((struct filename_representations *) fr)->display);
    g_free(((struct filename_representations *) fr)->collate_key);
    g_free(fr);
}

static void walk_recursive(const char *current_dir_string,
                           GDir *current_dir,
                           GTree *current_tree,
                           gboolean follow_symlinks,
                           GSList **errors)
{
    const gchar *basename;
    gchar *filename;

    while ((basename = g_dir_read_name(current_dir))) {
        filename = g_build_filename(current_dir_string, basename, NULL);
        if (g_file_test(filename, G_FILE_TEST_IS_DIR)) {
            struct filename_representations *fr = NULL;
            GError *err = NULL;
            GTree *sub_dir_tree = NULL;
            GDir *sub_dir;

            if (!follow_symlinks
                    && g_file_test(filename, G_FILE_TEST_IS_SYMLINK)) {
                goto next;
            }

            sub_dir = g_dir_open(filename, 0, &err);
            if (err) {
                *errors = g_slist_prepend(*errors, err);
                goto next;
            }
            fr = filename_representations_new(filename, FILETREE_DIR);
            sub_dir_tree = g_tree_new_full(compare_filenames, NULL,
                                           filename_representations_free,
                                           filetree_destroy);
            g_tree_insert(current_tree, fr, sub_dir_tree);
            walk_recursive(filename, sub_dir, sub_dir_tree,
                           follow_symlinks, errors);
            g_dir_close(sub_dir);
        } else if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
            struct filename_representations *fr;
            fr = filename_representations_new(filename, FILETREE_FILE);
            g_tree_insert(current_tree, fr, NULL);
        }
next:
        g_free(filename);
    }
}

/* root: directory in glib file name encoding */
Filetree filetree_init(char *roots[],
                       size_t roots_size,
                       gboolean recursive,
                       gboolean follow_symlinks,
                       gboolean no_sort,
                       GSList **errors)
{
    GTree *root_tree = g_tree_new_full(no_sort ? compare_insert_last
                                               : compare_filenames,
                                       NULL,
                                       filename_representations_free,
                                       filetree_destroy);
    size_t i;

    for (i = 0; i < roots_size; ++i) {
        GTree *sub_dir_tree;
        GDir *dir;
        GError *err;
        struct filename_representations *fr;

        if (!follow_symlinks
                && g_file_test(roots[i], G_FILE_TEST_IS_SYMLINK)) {
            continue;
        }

        if (g_file_test(roots[i], G_FILE_TEST_IS_DIR)) {
            if (!recursive) {
                *errors = g_slist_prepend(*errors, g_error_new(1, 1,
                                                               "Skipping '%s', is a directory", roots[i]));
                continue;
            }
            err = NULL;
            dir = g_dir_open(roots[i], 0, &err);
            if (err) {
                *errors = g_slist_prepend(*errors, err);
                err = NULL;
                continue;
            }
            fr = filename_representations_new(roots[i], FILETREE_DIR);
            sub_dir_tree = g_tree_new_full(compare_filenames, NULL,
                                           filename_representations_free,
                                           filetree_destroy);

            g_tree_insert(root_tree, fr, sub_dir_tree);
            walk_recursive(roots[i], dir, sub_dir_tree,
                           follow_symlinks, errors);
            g_dir_close(dir);
        } else if (g_file_test(roots[i], G_FILE_TEST_IS_REGULAR)) {
            fr = filename_representations_new(roots[i], FILETREE_FILE);
            g_tree_insert(root_tree, fr, NULL);
        } else {
            *errors = g_slist_prepend(*errors, g_error_new(1, 1,
                                                           "File '%s' not found", roots[i]));
        }
    }

    *errors = g_slist_reverse(*errors);

    return root_tree;
}

void filetree_print_error(gpointer user, gpointer user_data)
{
    GError *err = (GError *) user;
    (void) user_data;
    g_message("%s", err->message);
}

void filetree_free_error(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_error_free((GError *) user);
}

void filetree_get_file_size(gpointer user, gpointer user_data)
{
#if GLIB_CHECK_VERSION(2, 25, 0)
    GStatBuf stat_buf;
#else
    struct stat stat_buf;
#endif
    struct filename_list_node *fln = (struct filename_list_node *) user;
    guint64 *file_size;

    (void) user_data;
    fln->d = g_malloc(sizeof(guint64));
    file_size = (guint64 *) fln->d;
    if (g_stat(fln->fr->raw, &stat_buf)) {
        *file_size = 0;
    } else {
        *file_size = (guint64) stat_buf.st_size;
    }
}

void filetree_print_file_size(gpointer user, gpointer user_data)
{
    struct filename_list_node *fln = (struct filename_list_node *) user;
    guint64 *file_size = (guint64 *) fln->d;

    (void) user_data;
    g_print("%s", fln->fr->display);
    g_print(", %" G_GUINT64_FORMAT "\n", *file_size);
}

void filetree_free_list_entry(gpointer user, gpointer user_data)
{
    (void) user_data;
    g_free(((struct filename_list_node *) user)->d);
    g_free(user);
}


static gboolean print_tree_entries(gpointer key,
                                   gpointer value,
                                   gpointer data)
{
    int i;
    for (i = 0; i < GPOINTER_TO_INT(data); ++i) {
        putchar(' ');
    }
    g_print("%s", ((struct filename_representations*) key)->display);
    putchar('\n');
    if (value) {
        g_tree_foreach((Filetree) value,
                       print_tree_entries,
                       GINT_TO_POINTER(GPOINTER_TO_INT(data) + 2));
    }
    return FALSE;
}

static gboolean append_file_node(gpointer key,
                                 gpointer value,
                                 gpointer data)
{
    GSList **files = (GSList **) data;
    if (value) {
        g_tree_foreach((Filetree) value,
                       append_file_node,
                       files);
    } else {
        struct filename_list_node *new_file_node;
        new_file_node = g_malloc(sizeof(struct filename_list_node));
        new_file_node->fr = (struct filename_representations*) key;
        new_file_node->d = NULL;
        *files = g_slist_prepend(*files, new_file_node);
    }
    return FALSE;
}

void filetree_print(Filetree tree)
{
    g_tree_foreach(tree, print_tree_entries, GINT_TO_POINTER(0));
}

void filetree_destroy(Filetree tree)
{
    if (tree) {
        g_tree_destroy(tree);
    }
}

void filetree_file_list(Filetree tree, GSList **files)
{
    g_tree_foreach(tree, append_file_node, files);
    *files = g_slist_reverse(*files);
}

struct string_plus_length {
    char *string;
    size_t length;
};

static void get_shortest_string(gpointer user, gpointer user_data)
{
    struct filename_list_node *fln = (struct filename_list_node *) user;
    struct string_plus_length *spl = (struct string_plus_length *) user_data;
    size_t display_string_length = strlen(fln->fr->display);

    if (display_string_length < spl->length) {
        spl->string = fln->fr->display;
        spl->length = display_string_length;
    }
}

static void find_common_prefix(struct filename_list_node *fln, struct string_plus_length *cp)
{
    size_t i = 0;
    while (i < cp->length &&
           fln->fr->display[i] &&
           cp->string[i] &&
           fln->fr->display[i] == cp->string[i]) { ++i; }
    cp->length = i;
}

static void remove_prefix(struct filename_list_node *fln, size_t *len)
{
    char *startpos = fln->fr->display + *len;
    char *new_display;

    while (*startpos == G_DIR_SEPARATOR) ++startpos;

    new_display = g_strdup(startpos);
    g_free(fln->fr->display);
    fln->fr->display = new_display;
}

void filetree_remove_common_prefix(GSList *files) {
    struct string_plus_length spl = { NULL, (size_t) -1 }, common_prefix;
    char *shortest_dir_name;

    if (!files) return;
    g_slist_foreach(files, get_shortest_string, &spl);
    common_prefix.string = shortest_dir_name = g_path_get_dirname(spl.string);
    common_prefix.length = strlen(common_prefix.string);
    g_slist_foreach(files, (GFunc) find_common_prefix, &common_prefix);
    g_slist_foreach(files, (GFunc) remove_prefix, &common_prefix.length);

    g_free(shortest_dir_name);
}
#endif
