#ifndef __LANG_RES_OTA_H__
#define __LANG_RES_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "project_support_overlay.h"
#include "ota_tlvs_protocol.h" // IWYU pragma: keep

#if IS_SUPPORT_LANG_PACK
int start_lang_pack_upgrade(ota_img_info_t img_info, uint32_t *recv_size);
int end_lang_pack_upgrade(uint32_t img_id);
#else
static inline int start_lang_pack_upgrade(ota_img_info_t img_info, uint32_t *recv_size)
{
    (void)img_info;
    (void)recv_size;
    return -1;
}

static inline int end_lang_pack_upgrade(uint32_t img_id)
{
    (void)img_id;
    return -1;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
