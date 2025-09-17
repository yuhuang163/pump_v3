#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "hal_timer.h"
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "Elm.h"
#include "../src/velm.h"
#include "hal_cache.h"

#include "gpu_port.h"
#include "gpu_common.h"

extern uint32_t __test_evo_start_flash[];
extern uint32_t __test_evo_end_flash[];

typedef  struct appAttribs
{
    int winWidth;
    int winHeight;
    int frameCount;
};



int zoomOut = 0;
int scaleCount = 0;
static ElmBuffer buffer;
ElmHandle ego_handle = ELM_NULL_HANDLE;

static ElmBuffer fb;
struct appAttribs cmdAttrib = {640, 480, 1};

void cleanup(void)
{
    ElmDestroyBuffer(buffer);
    ElmDestroyObject(ego_handle);
    ElmTerminate();
}


static vg_lite_path_t path;
int evo_display_test_main_entry(int argc, const char * argv[])
{
    int status = 0;


    //initialize the Elem library
    status = ElmInitialize(480,480);
    if (!status)
    {

        printf("Elm init failed:");
        cleanup();
        return -1;
    }
    //creat an inner render buffer
    buffer = ElmCreateBuffer(cmdAttrib.winWidth, cmdAttrib.winHeight, ELM_BUFFER_FORMAT_RGBA8888);


    printf("start:%p,end:%p",__test_evo_start_flash,__test_evo_end_flash);

    ego_handle = ElmCreateObjectFromData(ELM_OBJECT_TYPE_EGO, __test_evo_start_flash,__test_evo_end_flash-__test_evo_start_flash);
    printf("--------open file finish--------");
    printf("RS: %dx%d frameCount:%d\n",cmdAttrib.winWidth, cmdAttrib.winHeight, cmdAttrib.frameCount);
    ElmClear(buffer, 0xFFFFFFFF, 0, 0, 0, 0, 1);
    static vg_lite_buffer_t * vg_fb;

    el_Obj_Buffer *raw_buff = (el_Obj_Buffer *)get_object(buffer);
    vg_fb=&raw_buff->buffer;

    status = ElmDraw(buffer, ego_handle);
    ElmFinish();
    // el_Obj_Buffer *raw_buff = (el_Obj_Buffer *)get_object(buffer);
    // gpu_save_raw("test.raw", &(raw_buff->buffer));
    vg_lite_save_raw("test.raw", vg_fb);


    cleanup();
    return 0;
}
