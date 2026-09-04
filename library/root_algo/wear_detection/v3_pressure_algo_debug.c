#include "v3_Defined.h"

extern SFEATURE_INFO tFeatureInfo[2];
extern SDATA_BUFFER tDataBuffer[2];
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
    unsigned short* uLeakDetectTime, 
    unsigned short* uDataBaselineCnt)
{
    if (uWaveSpikes != 0)
    {
        uWaveSpikes[0] = tFeatureInfo[0].uWaveSpikes;
        uWaveSpikes[1] = tFeatureInfo[1].uWaveSpikes;
    }

    if (fMinData != 0)
    {
        fMinData[0] = tFeatureInfo[0].fMinData;
        fMinData[1] = tFeatureInfo[1].fMinData;
    }

    if (uNormalWaveform != 0)
    {
        uNormalWaveform[0] = tFeatureInfo[0].uNormalWaveform;
        uNormalWaveform[1] = tFeatureInfo[1].uNormalWaveform;
    }

    if (uNoiseLevel != 0)
    {
        uNoiseLevel[0] = tFeatureInfo[0].uNoiseLevel;
        uNoiseLevel[1] = tFeatureInfo[1].uNoiseLevel;
    }

    if (fThreshold != 0)
    {
        fThreshold[0] = tFeatureInfo[0].fThreshold;
        fThreshold[1] = tFeatureInfo[1].fThreshold;
    }

    if (fFilteredData != 0)
    {
        unsigned int idx0 = (tDataBuffer[0].uCurrPos == 0) ? (WIN_SIZE - 1) : (tDataBuffer[0].uCurrPos - 1);
        unsigned int idx1 = (tDataBuffer[1].uCurrPos == 0) ? (WIN_SIZE - 1) : (tDataBuffer[1].uCurrPos - 1);

        fFilteredData[0] = tDataBuffer[0].fDataBuff[idx0];
        fFilteredData[1] = tDataBuffer[1].fDataBuff[idx1];
    }

    if (fRawSegMinPressure != 0)
    {
        fRawSegMinPressure[0] = tFeatureInfo[0].fRawSegMinPressure;
        fRawSegMinPressure[1] = tFeatureInfo[1].fRawSegMinPressure;
    }

    if (fRawSegThresholdAtMinPressure != 0)
    {
        fRawSegThresholdAtMinPressure[0] = tFeatureInfo[0].fRawSegThresholdAtMinPressure;
        fRawSegThresholdAtMinPressure[1] = tFeatureInfo[1].fRawSegThresholdAtMinPressure;
    }

    if (fFilteredSegMinPressure != 0)
    {
        fFilteredSegMinPressure[0] = tFeatureInfo[0].fFilteredSegMinPressure;
        fFilteredSegMinPressure[1] = tFeatureInfo[1].fFilteredSegMinPressure;
    }

    if (uWearDetSegMinCommitted != 0)
    {
        uWearDetSegMinCommitted[0] = tFeatureInfo[0].uWearDetSegMinCommitted;
        uWearDetSegMinCommitted[1] = tFeatureInfo[1].uWearDetSegMinCommitted;
    }
    if (uWearDetectTime != 0)
    {
        uWearDetectTime[0] = tFeatureInfo[0].uWearDetectTime;
        uWearDetectTime[1] = tFeatureInfo[1].uWearDetectTime;
    }
    if (uLeakDetectTime != 0)
    {
        uLeakDetectTime[0] = tFeatureInfo[0].uLeakDetectTime;
        uLeakDetectTime[1] = tFeatureInfo[1].uLeakDetectTime;
    }
    if (uDataBaselineCnt != 0)
    {
        uDataBaselineCnt[0] = tFeatureInfo[0].uDataBaselineCnt[0];
        uDataBaselineCnt[1] = tFeatureInfo[1].uDataBaselineCnt[1];
    }
}