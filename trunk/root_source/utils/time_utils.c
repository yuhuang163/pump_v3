/*
 * INCLUDE FILES
 *****************************************************************************************
 */
 #include "time_utils.h" // IWYU pragma: keep
 
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
 void timestamp_to_utc_time(uint32_t seconds, utc_time_t *utc_time)
 {
     const uint16_t days0[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
     const uint16_t days1[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
 
     utc_time_t time = {0};
     uint32_t days, secs;
     uint16_t year;
     const uint16_t *dayp;
 
     // 10957 is the days between 1970/1/1 and 2000/1/1
     if (seconds >= 10957 * 86400) {
         days = seconds / 86400 - 10957;
         secs = seconds % 86400;
     } else {
         days = 0;
         secs = 0;
     }
 
     time.second = secs % 60;
     secs /= 60;
     time.minute = secs % 60;
     secs /= 60;
     time.hour = secs;
 
     year = 2000;
     time.week = (days + 6) % 7;
 
     year += (days / 1461) * 4;
     days %= 1461;
 
     if (days >= 366)
         dayp = days1;
     else
         dayp = days0;
     
     if (days >= 366) {
         year += (days - 1) / 365;
         days = (days - 1) % 365;
     }
 
     time.month = days / 31 + 1;
     if (days >= dayp[time.month])
         time.month += 1;
 
     time.day = days - dayp[time.month - 1] + 1;
     time.year = year;
 
     memcpy(utc_time, &time, sizeof(utc_time_t));
 }
 
 uint32_t utc_time_to_timestamp(utc_time_t *utc_time)
 {
     uint16_t year = 0;
     uint32_t utc = 0;
 
     // 10957 is the days between 1970/1/1 and 2000/1/1
     year = (utc_time->year + 2000) % 100;
     utc  = 10957;
     utc += (year * 365 + (year + 3) / 4);
     utc += (367 * utc_time->month - 362) / 12 - (utc_time->month <= 2 ? 0 : ((year % 4 == 0) ? 1 : 2));
     utc += (utc_time->day - 1);
     utc *= 86400;
     utc += (utc_time->hour * 3600 + utc_time->minute * 60 + utc_time->second);
 
     return utc;
 }
 