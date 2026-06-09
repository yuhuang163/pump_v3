#ifndef __IMG_RES_OTA_H__
#define __IMG_RES_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "ota_tlvs_protocol.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int start_img_res_upgrade(ota_img_info_t img_info, uint32_t *recv_size);
int end_img_res_upgrade(uint32_t img_id);

#ifdef __cplusplus
}
#endif

#endif 