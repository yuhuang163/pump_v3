/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "resource_manager_api.h"
#if IS_SUPPORT_LANG_PACK
#include "ui_lang_pack.h"
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int on_lang_res_init(resource_manager_context_t *thiz)
{
    (void)thiz;
#if IS_SUPPORT_LANG_PACK
    return ui_lang_pack_probe();
#else
    return 0;
#endif
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_resource_hook(resource_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    ctx->hook = u_malloz(sizeof(*ctx->hook));
    EXCEPTION_CHECK_VOID(ctx->hook);
    ctx->hook->on_lang_res_init = on_lang_res_init;
}
