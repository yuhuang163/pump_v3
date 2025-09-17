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

#include "plat_addr_map.h"

#define STACK_SECTION_SIZE 0
#define HEAP_SECTION_SIZE 0

#if defined(CHIP_BEST1502X)
#define ROM1X_ORIGIN (0xc300000)
#define ROM1D_ORIGIN (0x2c300000)
#define ROM1_SIZE    (0xbb800)

#define RESV_RAM_TOP    (0x20140000)
#define RESV_RAM_SIZE   (0x10000)
#define RAM_RSV4ROM_ORIGIN (RESV_RAM_TOP - RESV_RAM_SIZE)
#elif defined(CHIP_BEST1503)
#define ROM1X_ORIGIN (0x00100000)
#define ROM1D_ORIGIN (0x22100000)
#define ROM1_SIZE    (0x8d400)

#define RESV_RAM_TOP    (0x20160000)
#define RESV_RAM_SIZE   (0x10000)
#define RAM_RSV4ROM_ORIGIN (RESV_RAM_TOP - RESV_RAM_SIZE)
#endif

#define ROMX_TO_ROMD(a)			((a) - ORIGIN(ROMX) + ORIGIN(ROMD))
#define ROMD_TO_ROMX(a)			((a) - ORIGIN(ROMD) + ORIGIN(ROMX))

/* Linker script to configure memory regions. */
MEMORY
{
  ROMX (rx)  : ORIGIN = ROM1X_ORIGIN, LENGTH = ROM1_SIZE
  ROMD (rx)  : ORIGIN = ROM1D_ORIGIN, LENGTH = ROM1_SIZE
  RAM_RSV4ROM (rwx) : ORIGIN = RAM_RSV4ROM_ORIGIN,LENGTH = RESV_RAM_SIZE
}

/* Library configurations */
/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __export_fn_rom
 *   __exidx_start
 *   __exidx_end
 *   __rom_etext
 *   __rom_data_start__
 *   __rom_data_end__
 *   __rom_bss_start__
 *   __rom_bss_end__
 *   __rom_HeapLimit
 *   __rom_StackLimit
 *   __rom_StackTop
 *   __rom_stack
 *   __Vectors_End
 *   __Vectors_Size
 *   __free_rom_ram
 *   __rom_end
 *   __free_rom
 *   __audio_const_rom_start
 *   __audio_const_rom_end
 *   __audio_const_rom_size
 */
ENTRY(rom1_entry)

SECTIONS
{
    .rom_text :
    {
        KEEP(*:main.o(.text.*rom1_entry*))
        KEEP(*:extern_functions.o(.text.*impure*))

        __bthost_text_start__ = .;
        KEEP(*:export_fn_reg_fn.o   (.text*))
        KEEP(*:bt_stack_mem.o   (.text*))
        KEEP(*:bt_common.o   (.text*))
        KEEP(*:bes_os.o   (.text*))
        KEEP(*:bth_patch_gaf.o   (.text*))
        KEEP(*:bth_patch_hci.o   (.text*))
        KEEP(*:bth_patch_l2cap.o   (.text*))
        KEEP(*:bth_patch_basic.o   (.text*))
        KEEP(*:bth_patch_btprf.o   (.text*))
        KEEP(*:cobuf.o   (.text*))
        KEEP(*:ecc_p192.o   (.text*))
        KEEP(*:l2cap.o   (.text*))
        KEEP(*:co_timer.o   (.text*))
        KEEP(*:co_lib.o   (.text*))
        KEEP(*:co_ppbuff.o   (.text*))
        KEEP(*:hci.o   (.text*))
        KEEP(*:hci_transport.o   (.text*))
        KEEP(*:bt_if.o   (.text*))
        KEEP(*:pbap_api.o   (.text*))
        KEEP(*:btgatt_api.o   (.text*))
        KEEP(*:besaud_api.o   (.text*))
        KEEP(*:dip_api.o   (.text*))
        KEEP(*:spp_api.o   (.text*))
        KEEP(*:a2dp_api.o   (.text*))
        KEEP(*:hfp_api.o   (.text*))
        KEEP(*:avrcp_api.o   (.text*))
        KEEP(*:me_api.o   (.text*))
        KEEP(*:map_api.o   (.text*))
        KEEP(*:pan_api.o   (.text*))
        KEEP(*:opp_api.o   (.text*))
        KEEP(*:hid_api.o   (.text*))
        KEEP(*:a2dp_decoder_lhdc_license.o   (.text*))
        KEEP(*:bap.o   (.text*))
        KEEP(*:iso.o   (.text*))
        KEEP(*:gaf_prf.o   (.text*))
        KEEP(*:ot_common.o   (.text*))
        KEEP(*:ots.o   (.text*))
        KEEP(*:tbs.o   (.text*))
        KEEP(*:mcs.o   (.text*))
        KEEP(*:tb_common.o   (.text*))
        KEEP(*:tbc.o   (.text*))
        KEEP(*:mcc.o   (.text*))
        KEEP(*:mc_common.o   (.text*))
        KEEP(*:otc.o   (.text*))
        KEEP(*:generic_audio.o   (.text*))
        KEEP(*:cas.o   (.text*))
        KEEP(*:cac.o   (.text*))
        KEEP(*:mics.o   (.text*))
        KEEP(*:vcc.o   (.text*))
        KEEP(*:vocs.o   (.text*))
        KEEP(*:aics.o   (.text*))
        KEEP(*:vcs.o   (.text*))
        KEEP(*:vocc.o   (.text*))
        KEEP(*:aicc.o   (.text*))
        KEEP(*:micc.o   (.text*))
        KEEP(*:bap_uc_test.o   (.text*))
        KEEP(*:gmas.o   (.text*))
        KEEP(*:gmac.o   (.text*))
        KEEP(*:csis.o   (.text*))
        KEEP(*:atc_csis_coordinator.o   (.text*))
        KEEP(*:csi_crypto.o   (.text*))
        KEEP(*:csisc.o   (.text*))
        KEEP(*:atc_csis_member.o   (.text*))
        KEEP(*:bass.o   (.text*))
        KEEP(*:public_broadcast_audio.o   (.text*))
        KEEP(*:bap_broadcast_assist.o   (.text*))
        KEEP(*:ascs.o   (.text*))
        KEEP(*:bap_broadcast_source.o   (.text*))
        KEEP(*:bap_unicast_server.o   (.text*))
        KEEP(*:pacc.o   (.text*))
        KEEP(*:pacs.o   (.text*))
        KEEP(*:bap_scan_delegator.o   (.text*))
        KEEP(*:bap_broadcast_sink.o   (.text*))
        KEEP(*:bap_unicast_client.o   (.text*))
        KEEP(*:asc_common.o   (.text*))
        KEEP(*:bap_broadcast_common.o   (.text*))
        KEEP(*:tmas.o   (.text*))
        KEEP(*:tmac.o   (.text*))
        KEEP(*:gaf_log.o   (.text*))
        KEEP(*:hac.o   (.text*))
        KEEP(*:has.o   (.text*))
        KEEP(*:smp.o   (.text*))
        KEEP(*:gatt.o   (.text*))
        KEEP(*:gap.o   (.text*))
        KEEP(*:sdp.o   (.text*))
        KEEP(*:pbap.o   (.text*))
        KEEP(*:bnep.o   (.text*))
        KEEP(*:obex.o   (.text*))
        KEEP(*:a2dp.o   (.text*))
        KEEP(*:hfp.o   (.text*))
        KEEP(*:avrcp.o   (.text*))
        KEEP(*:map.o   (.text*))
        KEEP(*:pan.o   (.text*))
        KEEP(*:rfcomm.o   (.text*))
        KEEP(*:opp.o   (.text*))
        KEEP(*:hid.o   (.text*))
        KEEP(*:btm.o   (.text*))
        __bthost_text_end__ = .;
        __bthost_text_size__ = __bthost_text_end__ - __bthost_text_start__;
        . = ALIGN(4);
    } > ROMX

    .rom_libc_text :
    {
        KEEP(*:export_utils_fn_rom.o   (.text*))
        KEEP(*:export_sys_fn_rom.o   (.text*))
        KEEP(*:export_nv_fn_rom.o   (.text*))
        *(.text)
        *(.text.*)
        . = ALIGN(4);
    }> ROMX

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ROMX

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > ROMX
    __exidx_end = .;
    . = ALIGN(4);

    .rodata (ROMX_TO_ROMD(__exidx_end)) : AT(__exidx_end)
    {
        __bthost_rodata_start__ = .;
        KEEP(*:bt_stack_mem.o   (.rodata*))
        KEEP(*:bt_common.o   (.rodata*))
        KEEP(*:bes_os.o   (.rodata*))
        KEEP(*:bth_patch_gaf.o   (.rodata*))
        KEEP(*:bth_patch_hci.o   (.rodata*))
        KEEP(*:bth_patch_l2cap.o   (.rodata*))
        KEEP(*:bth_patch_basic.o   (.rodata*))
        KEEP(*:bth_patch_btprf.o   (.rodata*))
        KEEP(*:cobuf.o   (.rodata*))
        KEEP(*:ecc_p192.o   (.rodata*))
        KEEP(*:l2cap.o   (.rodata*))
        KEEP(*:co_timer.o   (.rodata*))
        KEEP(*:co_lib.o   (.rodata*))
        KEEP(*:co_ppbuff.o   (.rodata*))
        KEEP(*:hci.o   (.rodata*))
        KEEP(*:hci_transport.o   (.rodata*))
        KEEP(*:bt_if.o   (.rodata*))
        KEEP(*:pbap_api.o   (.rodata*))
        KEEP(*:btgatt_api.o   (.rodata*))
        KEEP(*:besaud_api.o   (.rodata*))
        KEEP(*:dip_api.o   (.rodata*))
        KEEP(*:spp_api.o   (.rodata*))
        KEEP(*:a2dp_api.o   (.rodata*))
        KEEP(*:hfp_api.o   (.rodata*))
        KEEP(*:avrcp_api.o   (.rodata*))
        KEEP(*:me_api.o   (.rodata*))
        KEEP(*:map_api.o   (.rodata*))
        KEEP(*:pan_api.o   (.rodata*))
        KEEP(*:opp_api.o   (.rodata*))
        KEEP(*:hid_api.o   (.rodata*))
        KEEP(*:a2dp_decoder_lhdc_license.o   (.rodata*))
        KEEP(*:bap.o   (.rodata*))
        KEEP(*:iso.o   (.rodata*))
        KEEP(*:gaf_prf.o   (.rodata*))
        KEEP(*:ot_common.o   (.rodata*))
        KEEP(*:ots.o   (.rodata*))
        KEEP(*:tbs.o   (.rodata*))
        KEEP(*:mcs.o   (.rodata*))
        KEEP(*:tb_common.o   (.rodata*))
        KEEP(*:tbc.o   (.rodata*))
        KEEP(*:mcc.o   (.rodata*))
        KEEP(*:mc_common.o   (.rodata*))
        KEEP(*:otc.o   (.rodata*))
        KEEP(*:generic_audio.o   (.rodata*))
        KEEP(*:cas.o   (.rodata*))
        KEEP(*:cac.o   (.rodata*))
        KEEP(*:mics.o   (.rodata*))
        KEEP(*:vcc.o   (.rodata*))
        KEEP(*:vocs.o   (.rodata*))
        KEEP(*:aics.o   (.rodata*))
        KEEP(*:vcs.o   (.rodata*))
        KEEP(*:vocc.o   (.rodata*))
        KEEP(*:aicc.o   (.rodata*))
        KEEP(*:micc.o   (.rodata*))
        KEEP(*:bap_uc_test.o   (.rodata*))
        KEEP(*:gmas.o   (.rodata*))
        KEEP(*:gmac.o   (.rodata*))
        KEEP(*:csis.o   (.rodata*))
        KEEP(*:atc_csis_coordinator.o   (.rodata*))
        KEEP(*:csi_crypto.o   (.rodata*))
        KEEP(*:csisc.o   (.rodata*))
        KEEP(*:atc_csis_member.o   (.rodata*))
        KEEP(*:bass.o   (.rodata*))
        KEEP(*:public_broadcast_audio.o   (.rodata*))
        KEEP(*:bap_broadcast_assist.o   (.rodata*))
        KEEP(*:ascs.o   (.rodata*))
        KEEP(*:bap_broadcast_source.o   (.rodata*))
        KEEP(*:bap_unicast_server.o   (.rodata*))
        KEEP(*:pacc.o   (.rodata*))
        KEEP(*:pacs.o   (.rodata*))
        KEEP(*:bap_scan_delegator.o   (.rodata*))
        KEEP(*:bap_broadcast_sink.o   (.rodata*))
        KEEP(*:bap_unicast_client.o   (.rodata*))
        KEEP(*:asc_common.o   (.rodata*))
        KEEP(*:bap_broadcast_common.o   (.rodata*))
        KEEP(*:tmas.o   (.rodata*))
        KEEP(*:tmac.o   (.rodata*))
        KEEP(*:gaf_log.o   (.rodata*))
        KEEP(*:hac.o   (.rodata*))
        KEEP(*:has.o   (.rodata*))
        KEEP(*:smp.o   (.rodata*))
        KEEP(*:gatt.o   (.rodata*))
        KEEP(*:gap.o   (.rodata*))
        KEEP(*:sdp.o   (.rodata*))
        KEEP(*:pbap.o   (.rodata*))
        KEEP(*:bnep.o   (.rodata*))
        KEEP(*:obex.o   (.rodata*))
        KEEP(*:a2dp.o   (.rodata*))
        KEEP(*:hfp.o   (.rodata*))
        KEEP(*:avrcp.o   (.rodata*))
        KEEP(*:map.o   (.rodata*))
        KEEP(*:pan.o   (.rodata*))
        KEEP(*:rfcomm.o   (.rodata*))
        KEEP(*:opp.o   (.rodata*))
        KEEP(*:hid.o   (.rodata*))
        KEEP(*:btm.o   (.rodata*))
        KEEP(*:export_utils_fn_rom.o   (.rodata*))
        KEEP(*:export_fn_reg_fn.o   (.rodata*))
        KEEP(*:export_sys_fn_rom.o   (.rodata*))
        KEEP(*:export_nv_fn_rom.o   (.rodata*))


        KEEP(*(.note.gnu.build-id))
        *(.rodata)
        *(.rodata.*)
        __bthost_rodata_end__ = .;
        __bthost_rodata_size__ = __bthost_rodata_end__ - __bthost_rodata_start__;
    } > ROMD

    __rom_etext = (__exidx_end + SIZEOF(.rodata));

    .data (ORIGIN(RAM_RSV4ROM)) : AT (RAM_RSV4ROM_ORIGIN)
    {
        . = ALIGN(4);
        __bth_data_start__ = .;
        /* This is used by the startup in order to initialize the .data secion */
        KEEP(*:bt_stack_mem.o   (.data*))
        KEEP(*:bt_common.o   (.data*))
        KEEP(*:bes_os.o   (.data*))
        KEEP(*:bth_patch_gaf.o   (.data*))
        KEEP(*:bth_patch_hci.o   (.data*))
        KEEP(*:bth_patch_l2cap.o   (.data*))
        KEEP(*:bth_patch_basic.o   (.data*))
        KEEP(*:bth_patch_btprf.o   (.data*))
        KEEP(*:cobuf.o   (.data*))
        KEEP(*:ecc_p192.o   (.data*))
        KEEP(*:l2cap.o   (.data*))
        KEEP(*:co_timer.o   (.data*))
        KEEP(*:co_lib.o   (.data*))
        KEEP(*:co_ppbuff.o   (.data*))
        KEEP(*:hci.o   (.data*))
        KEEP(*:hci_transport.o   (.data*))
        KEEP(*:bt_if.o   (.data*))
        KEEP(*:pbap_api.o   (.data*))
        KEEP(*:btgatt_api.o   (.data*))
        KEEP(*:besaud_api.o   (.data*))
        KEEP(*:dip_api.o   (.data*))
        KEEP(*:spp_api.o   (.data*))
        KEEP(*:a2dp_api.o   (.data*))
        KEEP(*:hfp_api.o   (.data*))
        KEEP(*:avrcp_api.o   (.data*))
        KEEP(*:me_api.o   (.data*))
        KEEP(*:map_api.o   (.data*))
        KEEP(*:pan_api.o   (.data*))
        KEEP(*:opp_api.o   (.data*))
        KEEP(*:hid_api.o   (.data*))
        KEEP(*:a2dp_decoder_lhdc_license.o   (.data*))
        KEEP(*:bap.o   (.data*))
        KEEP(*:iso.o   (.data*))
        KEEP(*:gaf_prf.o   (.data*))
        KEEP(*:ot_common.o   (.data*))
        KEEP(*:ots.o   (.data*))
        KEEP(*:tbs.o   (.data*))
        KEEP(*:mcs.o   (.data*))
        KEEP(*:tb_common.o   (.data*))
        KEEP(*:tbc.o   (.data*))
        KEEP(*:mcc.o   (.data*))
        KEEP(*:mc_common.o   (.data*))
        KEEP(*:otc.o   (.data*))
        KEEP(*:generic_audio.o   (.data*))
        KEEP(*:cas.o   (.data*))
        KEEP(*:cac.o   (.data*))
        KEEP(*:mics.o   (.data*))
        KEEP(*:vcc.o   (.data*))
        KEEP(*:vocs.o   (.data*))
        KEEP(*:aics.o   (.data*))
        KEEP(*:vcs.o   (.data*))
        KEEP(*:vocc.o   (.data*))
        KEEP(*:aicc.o   (.data*))
        KEEP(*:micc.o   (.data*))
        KEEP(*:bap_uc_test.o   (.data*))
        KEEP(*:gmas.o   (.data*))
        KEEP(*:gmac.o   (.data*))
        KEEP(*:csis.o   (.data*))
        KEEP(*:atc_csis_coordinator.o   (.data*))
        KEEP(*:csi_crypto.o   (.data*))
        KEEP(*:csisc.o   (.data*))
        KEEP(*:atc_csis_member.o   (.data*))
        KEEP(*:bass.o   (.data*))
        KEEP(*:public_broadcast_audio.o   (.data*))
        KEEP(*:bap_broadcast_assist.o   (.data*))
        KEEP(*:ascs.o   (.data*))
        KEEP(*:bap_broadcast_source.o   (.data*))
        KEEP(*:bap_unicast_server.o   (.data*))
        KEEP(*:pacc.o   (.data*))
        KEEP(*:pacs.o   (.data*))
        KEEP(*:bap_scan_delegator.o   (.data*))
        KEEP(*:bap_broadcast_sink.o   (.data*))
        KEEP(*:bap_unicast_client.o   (.data*))
        KEEP(*:asc_common.o   (.data*))
        KEEP(*:bap_broadcast_common.o   (.data*))
        KEEP(*:tmas.o   (.data*))
        KEEP(*:tmac.o   (.data*))
        KEEP(*:gaf_log.o   (.data*))
        KEEP(*:hac.o   (.data*))
        KEEP(*:has.o   (.data*))
        KEEP(*:smp.o   (.data*))
        KEEP(*:gatt.o   (.data*))
        KEEP(*:gap.o   (.data*))
        KEEP(*:sdp.o   (.data*))
        KEEP(*:pbap.o   (.data*))
        KEEP(*:bnep.o   (.data*))
        KEEP(*:obex.o   (.data*))
        KEEP(*:a2dp.o   (.data*))
        KEEP(*:hfp.o   (.data*))
        KEEP(*:avrcp.o   (.data*))
        KEEP(*:map.o   (.data*))
        KEEP(*:pan.o   (.data*))
        KEEP(*:rfcomm.o   (.data*))
        KEEP(*:opp.o   (.data*))
        KEEP(*:hid.o   (.data*))
        KEEP(*:btm.o   (.data*))
        KEEP(*:export_utils_fn_rom.o   (.data*))
        KEEP(*:export_fn_reg_fn.o   (.data*))
        KEEP(*:export_sys_fn_rom.o   (.data*))
        KEEP(*:export_nv_fn_rom.o   (.data*))

        KEEP(*(.data))
        KEEP(*(.data.*))

         __bth_data_end__ = .;
        __bthost_data_size__ = __bth_data_end__ - __bth_data_start__;
        /* This is used by the startup in order to initialize the .data secion */
    } >RAM_RSV4ROM

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bth_bss_start__ = .;
        KEEP(*:bt_stack_mem.o   (.bss*))
        KEEP(*:bt_common.o   (.bss*))
        KEEP(*:bes_os.o   (.bss*))
        KEEP(*:bth_patch_gaf.o   (.bss*))
        KEEP(*:bth_patch_hci.o   (.bss*))
        KEEP(*:bth_patch_l2cap.o   (.bss*))
        KEEP(*:bth_patch_basic.o   (.bss*))
        KEEP(*:bth_patch_btprf.o   (.bss*))
        KEEP(*:cobuf.o   (.bss*))
        KEEP(*:ecc_p192.o   (.bss*))
        KEEP(*:l2cap.o   (.bss*))
        KEEP(*:co_timer.o   (.bss*))
        KEEP(*:co_lib.o   (.bss*))
        KEEP(*:co_ppbuff.o   (.bss*))
        KEEP(*:hci.o   (.bss*))
        KEEP(*:hci_transport.o   (.bss*))
        KEEP(*:bt_if.o   (.bss*))
        KEEP(*:pbap_api.o   (.bss*))
        KEEP(*:btgatt_api.o   (.bss*))
        KEEP(*:besaud_api.o   (.bss*))
        KEEP(*:dip_api.o   (.bss*))
        KEEP(*:spp_api.o   (.bss*))
        KEEP(*:a2dp_api.o   (.bss*))
        KEEP(*:hfp_api.o   (.bss*))
        KEEP(*:avrcp_api.o   (.bss*))
        KEEP(*:me_api.o   (.bss*))
        KEEP(*:map_api.o   (.bss*))
        KEEP(*:pan_api.o   (.bss*))
        KEEP(*:opp_api.o   (.bss*))
        KEEP(*:hid_api.o   (.bss*))
        KEEP(*:a2dp_decoder_lhdc_license.o   (.bss*))
        KEEP(*:bap.o   (.bss*))
        KEEP(*:iso.o   (.bss*))
        KEEP(*:gaf_prf.o   (.bss*))
        KEEP(*:ot_common.o   (.bss*))
        KEEP(*:ots.o   (.bss*))
        KEEP(*:tbs.o   (.bss*))
        KEEP(*:mcs.o   (.bss*))
        KEEP(*:tb_common.o   (.bss*))
        KEEP(*:tbc.o   (.bss*))
        KEEP(*:mcc.o   (.bss*))
        KEEP(*:mc_common.o   (.bss*))
        KEEP(*:otc.o   (.bss*))
        KEEP(*:generic_audio.o   (.bss*))
        KEEP(*:cas.o   (.bss*))
        KEEP(*:cac.o   (.bss*))
        KEEP(*:mics.o   (.bss*))
        KEEP(*:vcc.o   (.bss*))
        KEEP(*:vocs.o   (.bss*))
        KEEP(*:aics.o   (.bss*))
        KEEP(*:vcs.o   (.bss*))
        KEEP(*:vocc.o   (.bss*))
        KEEP(*:aicc.o   (.bss*))
        KEEP(*:micc.o   (.bss*))
        KEEP(*:bap_uc_test.o   (.bss*))
        KEEP(*:gmas.o   (.bss*))
        KEEP(*:gmac.o   (.bss*))
        KEEP(*:csis.o   (.bss*))
        KEEP(*:atc_csis_coordinator.o   (.bss*))
        KEEP(*:csi_crypto.o   (.bss*))
        KEEP(*:csisc.o   (.bss*))
        KEEP(*:atc_csis_member.o   (.bss*))
        KEEP(*:bass.o   (.bss*))
        KEEP(*:public_broadcast_audio.o   (.bss*))
        KEEP(*:bap_broadcast_assist.o   (.bss*))
        KEEP(*:ascs.o   (.bss*))
        KEEP(*:bap_broadcast_source.o   (.bss*))
        KEEP(*:bap_unicast_server.o   (.bss*))
        KEEP(*:pacc.o   (.bss*))
        KEEP(*:pacs.o   (.bss*))
        KEEP(*:bap_scan_delegator.o   (.bss*))
        KEEP(*:bap_broadcast_sink.o   (.bss*))
        KEEP(*:bap_unicast_client.o   (.bss*))
        KEEP(*:asc_common.o   (.bss*))
        KEEP(*:bap_broadcast_common.o   (.bss*))
        KEEP(*:tmas.o   (.bss*))
        KEEP(*:tmac.o   (.bss*))
        KEEP(*:gaf_log.o   (.bss*))
        KEEP(*:hac.o   (.bss*))
        KEEP(*:has.o   (.bss*))
        KEEP(*:smp.o   (.bss*))
        KEEP(*:gatt.o   (.bss*))
        KEEP(*:gap.o   (.bss*))
        KEEP(*:sdp.o   (.bss*))
        KEEP(*:pbap.o   (.bss*))
        KEEP(*:bnep.o   (.bss*))
        KEEP(*:obex.o   (.bss*))
        KEEP(*:a2dp.o   (.bss*))
        KEEP(*:hfp.o   (.bss*))
        KEEP(*:avrcp.o   (.bss*))
        KEEP(*:map.o   (.bss*))
        KEEP(*:pan.o   (.bss*))
        KEEP(*:rfcomm.o   (.bss*))
        KEEP(*:opp.o   (.bss*))
        KEEP(*:hid.o   (.bss*))
        KEEP(*:btm.o   (.bss*))
        KEEP(*:export_utils_fn_rom.o   (.bss*))
        KEEP(*:export_fn_reg_fn.o   (.bss*))
        KEEP(*:export_sys_fn_rom.o   (.bss*))
        KEEP(*:export_nv_fn_rom.o   (.bss*))


        KEEP(*(.bss))
        KEEP(*(.bss.*))
        __bth_bss_end__ = .;
        __bthost_bss_size__ = __bth_bss_end__ - __bth_bss_start__;
    } > RAM_RSV4ROM

    .rom_heap (NOLOAD) :
    {
        . = ALIGN(4);
        __rom_HeapBase = .;
        __end__ = .;
        end = __end__;
        . = HEAP_SECTION_SIZE;
        __rom_HeapLimit = .;
    } > RAM_RSV4ROM

    .scratch_buf (NOLOAD) :
    {
        . = ALIGN(4);
        *(.scratch_buf)
    } > RAM_RSV4ROM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later
     */

    .rom_stack_dummy (COPY) :
    {
        . = STACK_SECTION_SIZE;
        . = ALIGN(8);
    } > RAM_RSV4ROM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __rom_StackTop = ORIGIN(RAM_RSV4ROM) + LENGTH(RAM_RSV4ROM);
    __rom_StackLimit = __rom_StackTop - SIZEOF(.rom_stack_dummy);
    PROVIDE(__rom_stack = __rom_StackTop);

    __StackTop = __rom_StackTop;
    PROVIDE(__stack = __rom_StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__rom_StackLimit >= __rom_HeapLimit, "region RAM overflowed with stack")
    __free_rom_ram = __rom_StackLimit - __rom_HeapLimit;

    __rom_end = __rom_etext;

    ASSERT(__rom_end < (ORIGIN(ROMX) + LENGTH(ROMX)), "region ROM overflowed")
    __free_rom = ((ORIGIN(ROMX) + LENGTH(ROMX))) - __rom_end;

    /DISCARD/ :
    {
        *(.interp)
        *(.hash)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.version)
        *(.gnu.version_d)
        *(.gnu.version_r)
        *(.rela.plt)
        *(.rela.data.rel.ro*)
        *(.rela.gnu.linkonce.d.rel.ro*)
        *(.rela.*)
        *(.plt)
        *(.dynamic)
    }
}

