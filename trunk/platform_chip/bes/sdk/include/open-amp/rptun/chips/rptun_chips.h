#ifndef __RPTUN_CHIPS_H__
#define __RPTUN_CHIPS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CHIP_BEST1600
#include "best1600/rptun_best1600.h"
#endif

#ifdef CHIP_BEST1502X
#include "best1502x/rptun_best1502x.h"
#endif

#ifdef CHIP_BEST1700
#include "best1700/rptun_best1700.h"
#endif

#ifdef SYSLOG_RPMSG
int syslog_rpmsg_init(void);
#endif
//other chips
#ifdef __cplusplus
}
#endif

#endif /* __RPTUN_CHIPS_H__ */
