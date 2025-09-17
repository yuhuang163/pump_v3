#include <stdio.h>
#include "vg_lite_util.h"

int cleanup(vg_lite_path_t * path, vg_lite_buffer_t *buffer){
	if (buffer->handle != NULL) {
        // Free the offscreen framebuffer memory.
        vg_lite_free(buffer);
    }
    vg_lite_clear_path(path);
    vg_lite_close();
	return 0;
}