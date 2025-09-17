/***************************************************************************
 *
 * Copyright 2015-2023 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BT_PATCH_GAF_H__
#define __BT_PATCH_GAF_H__

#include "gatt_service.h"
#include "bap_unicast_client.h"
#include "bap_broadcast_common.h"

/// Function table
extern uint32_t g_patch_tbl_gaf[];

/// Function enumeration
typedef enum
{
    FUNC_ID_gaf_prf_alloc_or_get_con_lid_with_gap_conidx,
    FUNC_ID_mcs_prepare_service_attr_list,
    FUNC_ID_tbs_prepare_service_attr_list,
    FUNC_ID_ots_prepare_service_attr_list,
    FUNC_ID_csis_prepare_service_attr_list,
    FUNC_ID_ascs_send_notify,
    FUNC_ID_asc_common_ase_state_changes_check,
    FUNC_ID_ascc_write_ase_cp,
    FUNC_ID_bap_uc_srv_before_set_ase_state,
    FUNC_ID_bap_uc_srv_queued_disconnect_cis,
    FUNC_ID_bap_uc_srv_iso_cb_cis_conn_request,
    FUNC_ID_bap_uc_cli_before_set_ase_state,
    FUNC_ID_bap_uc_cli_prepare_cis_hdl_to_create_cis,
    FUNC_ID_bap_uc_cli_queued_create_cis,
    FUNC_ID_bap_uc_cli_queued_disconnect_cis,
    FUNC_ID_bap_uc_cli_loop_prepare_ase_cp_wr_buf,
    FUNC_ID_bap_uc_cli_before_cfg_uc_stream_grp,
    FUNC_ID_bap_bc_scan_deleg_is_bc_rx_src_proc_busy,
    FUNC_ID_bap_scan_deleg_get_total_sub_grp_bis_sync_req_bf,
    FUNC_ID_bap_scan_deleg_before_send_bc_rx_state_ntf,
    FUNC_ID_mcs_gatt_server_callback,
    FUNC_ID_ots_gatt_server_callback,
    FUNC_ID_tbs_gatt_server_callback,
    FUNC_ID_csis_gatt_server_callback,
    FUNC_ID_ascs_gatt_server_callback,
    FUNC_ID_bass_gatt_server_callback,
    FUNC_ID_pacs_gatt_server_callback,
    FUNC_ID_aics_gatt_server_callback,
    FUNC_ID_vocs_gatt_server_callback,
    FUNC_ID_vcs_gatt_server_callback,
    FUNC_ID_mics_gatt_server_callback,
    FUNC_ID_cas_gatt_server_callback,
    FUNC_ID_gmas_gatt_server_callback,
    FUNC_ID_has_gatt_server_callback,
    FUNC_ID_tmas_gatt_server_callback,
    FUNC_ID_mcc_gatt_callback,
    FUNC_ID_otc_gatt_callback,
    FUNC_ID_tbc_gatt_callback,
    FUNC_ID_aicc_gatt_callback,
    FUNC_ID_micc_gatt_callback,
    FUNC_ID_vcc_gatt_callback,
    FUNC_ID_vocc_gatt_callback,
    FUNC_ID_csisc_gatt_callback,
    FUNC_ID_bap_bc_assist_gatt_callback,
    FUNC_ID_ascc_gatt_callback,
    FUNC_ID_pacc_gatt_callback,
    FUNC_ID_cac_gatt_callback,
    FUNC_ID_gmac_gatt_callback,
    FUNC_ID_hac_gatt_callback,
    FUNC_ID_tmac_gatt_callback,
    FUNC_ID_gen_aud_codec_cfg_check_param,
    FUNC_ID_gen_aud_codec_capa_check_param,
    FUNC_ID_gaf_log_func_internal,
    FUNC_ID_gaf_log_dump_internal,
    FUNC_ID_gen_aud_codec_is_id_valid,
    FUNC_ID_bap_uc_srv_bap_stream_event_callback,
    FUNC_ID_bap_uc_cli_stream_event_callback,

    FUNC_ID_GAF_PATCH_NUM_MAX,
} bt_patch_gaf_func_enum_t;

#if defined (BUILD_BTH_ROM)
/// Function extern
extern uint8_t gaf_prf_alloc_or_get_con_lid_with_gap_conidx(uint8_t conidx, uint16_t event);
extern uint16_t mcs_prepare_service_attr_list(bool is_gmcs, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len, uint8_t transfer_lid);
extern uint16_t tbs_prepare_service_attr_list(bool is_gtbs, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
extern uint16_t ots_prepare_service_attr_list(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
extern uint16_t csis_prepare_service_attr_list(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
extern uint16_t ascs_send_notify(uint8_t con_lid, uint8_t ase_inst_id, const uint8_t *character, const uint8_t *ntf_buf, uint16_t buf_len);
extern bool asc_common_ase_state_changes_check(uint8 opcode, uint8_t direction, uint8_t old_state, uint8_t new_state);
extern uint16_t ascc_write_ase_cp(uint8_t con_lid, uint8_t *val, uint8_t val_len, uint16_t ase_op_bf);
extern uint16_t bap_uc_srv_before_set_ase_state(void *p_ase, uint8_t ase_state, bool *continue_handle);
extern uint16_t bap_uc_srv_queued_disconnect_cis(uint16_t cis_hdl);
extern void bap_uc_srv_iso_cb_cis_conn_request(uint8_t con_lid, uint8_t cig_id, uint8_t cis_id, uint16_t cis_hdl);
extern uint16_t bap_uc_cli_before_set_ase_state(void *p_ase, uint8_t ase_state, const uint8_t *ase_state_param, uint8_t param_len, bool *continue_handle);
extern uint16_t bap_uc_cli_prepare_cis_hdl_to_create_cis(uint8_t con_lid, uint16_t *cis_hdl_list, uint8_t *cis_count);
extern uint16_t bap_uc_cli_queued_create_cis(uint8_t con_lid, uint8_t cis_count, uint16_t *p_cis_hdl);
extern uint16_t bap_uc_cli_queued_disconnect_cis(uint16_t cis_hdl);
extern uint16_t bap_uc_cli_loop_prepare_ase_cp_wr_buf(uint8_t op_code, uint8_t ase_index, const bap_uc_cli_ase_op_param_t *p_ase_param,
                                                             uint8_t *wr_cp_buf_len, uint8_t **p_wr_cp_buf);
extern uint16_t bap_uc_cli_before_cfg_uc_stream_grp(void *p_grp, uint8_t ase_num, const bap_uc_cli_ase_op_param_t *p_ase_param, bool *continue_handle);
extern bool bap_bc_scan_deleg_is_bc_rx_src_proc_busy(void);
extern uint32_t bap_scan_deleg_get_total_sub_grp_bis_sync_req_bf(uint8_t num_sub_grp, bap_bc_sub_grp_t *p_sub_grp);
extern uint16_t bap_scan_deleg_before_send_bc_rx_state_ntf(void *p_rx_src, bool *continue_handle);
extern int mcs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int ots_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int tbs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int csis_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int ascs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int bass_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int pacs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int aics_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int vocs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int vcs_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int mics_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int cas_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int gmas_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int has_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int tmas_gatt_server_callback(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
extern int mcc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int otc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int tbc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int aicc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int micc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int vcc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int vocc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int csisc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int bap_bc_assist_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int ascc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int pacc_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int cac_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int gmac_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int hac_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern int tmac_gatt_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
extern uint16_t gen_aud_codec_cfg_check_param(const uint8_t *p_codec_id, const gen_aud_codec_cfg_param_t *p_param, uint16_t *p_ltv_len);
extern uint16_t gen_aud_codec_capa_check_param(const uint8_t *p_codec_id, const gen_aud_codec_capa_param_t *p_param, uint16_t *p_ltv_len);
extern int gaf_log_func_internal(uint8_t log_lvl, const char *format, va_list var_args);
extern int gaf_log_dump_internal(const char *format, unsigned int size, const void *buffer, unsigned int count);
extern bool gen_aud_codec_is_id_valid(const uint8_t *p_codec_id);
extern int bap_uc_srv_bap_stream_event_callback(uintptr_t group_id, bap_event_t event, bap_event_param_t param);
extern int bap_uc_cli_stream_event_callback(uintptr_t group_id, bap_event_t event, bap_event_param_t param);

/// Function pointer typedef
typedef uint8_t (*gaf_prf_alloc_or_get_con_lid_with_gap_conidx_func_t)(uint8_t conidx, uint16_t event);
typedef uint16_t (*mcs_prepare_service_attr_list_func_t)(bool is_gmcs, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len, uint8_t transfer_lid);
typedef uint16_t (*tbs_prepare_service_attr_list_func_t)(bool is_gtbs, gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
typedef uint16_t (*ots_prepare_service_attr_list_func_t)(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
typedef uint16_t (*csis_prepare_service_attr_list_func_t)(gatt_attribute_t **pp_attr_list, uint8_t *p_attr_list_len);
typedef uint16_t (*ascs_send_notify_func_t)(uint8_t con_lid, uint8_t ase_inst_id, const uint8_t *character, const uint8_t *ntf_buf, uint16_t buf_len);
typedef bool (*asc_common_ase_state_changes_check_func_t)(uint8 opcode, uint8_t direction, uint8_t old_state, uint8_t new_state);
typedef uint16_t (*ascc_write_ase_cp_func_t)(uint8_t con_lid, uint8_t *val, uint8_t val_len, uint16_t ase_op_bf);
typedef uint16_t (*bap_uc_srv_before_set_ase_state_func_t)(void *p_ase, uint8_t ase_state, bool *continue_handle);
typedef uint16_t (*bap_uc_srv_queued_disconnect_cis_func_t)(uint16_t cis_hdl);
typedef void (*bap_uc_srv_iso_cb_cis_conn_request_func_t)(uint8_t con_lid, uint8_t cig_id, uint8_t cis_id, uint16_t cis_hdl);
typedef uint16_t (*bap_uc_cli_before_set_ase_state_func_t)(void *p_ase, uint8_t ase_state, const uint8_t *ase_state_param, uint8_t param_len, bool *continue_handle);
typedef uint16_t (*bap_uc_cli_prepare_cis_hdl_to_create_cis_func_t)(uint8_t con_lid, uint16_t *cis_hdl_list, uint8_t *cis_count);
typedef uint16_t (*bap_uc_cli_queued_create_cis_func_t)(uint8_t con_lid, uint8_t cis_count, uint16_t *p_cis_hdl);
typedef uint16_t (*bap_uc_cli_queued_disconnect_cis_func_t)(uint16_t cis_hdl);
typedef uint16_t (*bap_uc_cli_loop_prepare_ase_cp_wr_buf_func_t)(uint8_t op_code, uint8_t ase_index, const bap_uc_cli_ase_op_param_t *p_ase_param,
                                                                 uint8_t *wr_cp_buf_len, uint8_t **p_wr_cp_buf);
typedef uint16_t (*bap_uc_cli_before_cfg_uc_stream_grp_func_t)(void *p_grp, uint8_t ase_num, const bap_uc_cli_ase_op_param_t *p_ase_param, bool *continue_handle);
typedef bool (*bap_bc_scan_deleg_is_bc_rx_src_proc_busy_func_t)(void);
typedef uint32_t (*bap_scan_deleg_get_total_sub_grp_bis_sync_req_bf_func_t)(uint8_t num_sub_grp, bap_bc_sub_grp_t *p_sub_grp);
typedef uint16_t (*bap_scan_deleg_before_send_bc_rx_state_ntf_func_t)(void *p_rx_src, bool *continue_handle);
typedef int (*mcs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*ots_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*tbs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*csis_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*ascs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*bass_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*pacs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*aics_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*vocs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*vcs_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*mics_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*cas_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*gmas_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*has_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*tmas_gatt_server_callback_func_t)(gatt_svc_t *p_svc, gatt_server_event_t event, gatt_server_callback_param_t param);
typedef int (*mcc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*otc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*tbc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*aicc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*micc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*vcc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*vocc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*csisc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*bap_bc_assist_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*ascc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*pacc_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*cac_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*gmac_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*hac_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef int (*tmac_gatt_callback_func_t)(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
typedef uint16_t (*gen_aud_codec_cfg_check_param_func_t)(const uint8_t *p_codec_id, const gen_aud_codec_cfg_param_t *p_param, uint16_t *p_ltv_len);
typedef uint16_t (*gen_aud_codec_capa_check_param_func_t)(const uint8_t *p_codec_id, const gen_aud_codec_capa_param_t *p_param, uint16_t *p_ltv_len);
typedef int (*gaf_log_func_internal_func_t)(uint8_t log_lvl, const char *format, va_list var_args);
typedef int (*gaf_log_dump_internal_func_t)(const char *format, unsigned int size, const void *buffer, unsigned int count);
typedef bool (*gen_aud_codec_is_id_valid_func_t)(const uint8_t *p_codec_id);
typedef int (*bap_uc_srv_bap_stream_event_callback_func_t)(uintptr_t group_id, bap_event_t event, bap_event_param_t param);
typedef int (*bap_uc_cli_stream_event_callback_func_t)(uintptr_t group_id, bap_event_t event, bap_event_param_t param);

#endif /* BUILD_BTH_ROM */

#endif /* __BT_PATCH_GAF_H__ */