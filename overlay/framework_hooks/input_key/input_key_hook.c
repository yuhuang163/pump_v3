/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h"  // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int input_keyevent_route(struct key_input_context *ctx, uint16_t keycode, uint16_t keytype, int flag)
{
    /// 处理按键事件
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_input_key_hook(key_input_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __key_input_hook));
    ctx->hook->input_keyevent_route = input_keyevent_route;
    ctx->hook->get_key_code_name = get_input_key_code_name;
}
