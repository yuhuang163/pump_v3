/*
 * INCLUDE FILES
 *****************************************************************************************
 */
 #include "root_common_api.h" 

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
 
 /*
  * GLOBAL FUNCTION DEFINITIONS
  *****************************************************************************************
  */

 void __system_reboot(const char *reason)
 {
     const char *rstr = reason ? reason : "null";
     if (strncmp(rstr, __REBOOT_REASON_FACTORY_RESET, strlen(rstr)) != 0) {
         LOGE("==>: reason=%s\n", rstr);
         SEND_BROADCAST_SYNC_ARGS2(ID_BROADCAST_SYS_POWER_STATE_CHANGE, ID_BROADCAST_SYS_POWER_OFF, (uint32_t)reason);
     } else {
         LOGE("==>reason=%s\n", rstr);
     }
     
     for (int i = 0; i < 5; i++) {
         LOGE("wait task queue idle: %ds", i);
         int rc0 = is_handler_post_queue_busy();
         int rc1 = is_broadcast_queue_busy();
         if (!rc0 && !rc1) {
             break;
         }
         msleep(1000);
     }
 
     LOGE("-->go lowlevel reboot: %s", rstr);
     platform_system_reboot();
 }
 
 