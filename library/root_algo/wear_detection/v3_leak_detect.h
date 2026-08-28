#ifndef V3_LEAK_DETECT_H
#define V3_LEAK_DETECT_H

#include "v3_Defined.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 漏气检测（由 WearDetectionDetermination 在泵周期提交后调用）
 * 编码：0 未佩戴；1 已佩戴无漏；2 小漏；3 大漏。不改 uWearStatus。
 * LeakDetectReset：Init / 换档换模式
 * LeakDetectUpdate：写入谷值并按佩戴/模式更新 uLeakStatus
 */
void LeakDetectReset(SFEATURE_INFO* tFeatureInfoTemp);
void LeakDetectUpdate(SFEATURE_INFO* tFeatureInfoTemp, SDEVICE_INFO* tDeviceInfoTemp);

#ifdef __cplusplus
}
#endif

#endif


#ifdef __cplusplus
}
#endif
