#pragma once
#ifndef V3_PRESSURE_ALGO_DEBUG_H
#define V3_PRESSURE_ALGO_DEBUG_H

void GetWearDetectionDebugInfoAlg(unsigned int* uWaveSpikes,
    float* fMinData,
    unsigned int* uNormalWaveform,
    unsigned int* uNoiseLevel,
    float* fThreshold,
    float* fFilteredData,
    float* fRawSegMinPressure,
    float* fRawSegThresholdAtMinPressure,
    float* fFilteredSegMinPressure,
    unsigned char* uWearDetSegMinCommitted, 
    unsigned short* uWearDetectTime,
    unsigned short* uLeakDetectTime);

#endif /* V3_PRESSURE_ALGO_DEBUG_H */