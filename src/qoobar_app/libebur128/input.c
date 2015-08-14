/* See LICENSE file for copyright and license details. */
#include "input.h"

#ifdef WITH_DECODING

#include <stdio.h>

#include "input_gstreamer.h"
#include "input_ffmpeg.h"

struct input_ops* plugin_ops;

int input_init(int forced_plugin)
{
    int plugin_found = 0;

#ifdef USE_FFMPEG
    if (forced_plugin == PLUGIN_FFMPEG) {
        plugin_ops = &ffmpeg_ip_ops;
        plugin_found = 1;
    }
#endif

    if (forced_plugin == PLUGIN_GSTREAMER) {
        plugin_ops = &gstreamer_ip_ops;
        plugin_found = 1;
    }
    if (!plugin_found) {
        fprintf(stderr, "Warning: no plugins found!\n");
        return 1;
    }
    if (plugin_found) {
        plugin_ops->init_library();
    }
    return 0;
}

int input_deinit(void)
{
    plugin_ops->exit_library();
    return 0;
}

struct input_ops* input_get_ops()
{
    return plugin_ops;
}

#endif
