/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "app_voice_assist_fir_lms.h"
#include "anc_ff_fir_lms.h"
#include "anc_assist.h"

#include "audio_dump.h"
#include "app_utils.h"

static ANCFFFirLmsSt * fir_st= NULL;
static ANC_FF_FIR_LMS_CFG_T cfg ={
    .max_cnt = 100,
    .period_cnt = 5,
};
static int stop_flag = 0;


static int32_t _voice_assist_fir_lms_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_fir_lms_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_FIR_LMS, _voice_assist_fir_lms_callback);
    return 0;
}

int32_t app_voice_assist_fir_lms_reset(void){
    anc_ff_fir_lms_reset(fir_st,0);
    return 0;
}

void set_fixed_fir_filter(void){
    anc_ff_fir_lms_reset(fir_st,1);
}


int32_t app_voice_assist_fir_lms_open(void)
{
    TRACE(0, "[%s] oneshot adapt anc start stream", __func__);
    stop_flag = 0;
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
    
    fir_st = anc_ff_fir_lms_create(16000,120,&cfg);
    app_anc_assist_open(ANC_ASSIST_USER_FIR_LMS);
    return 0;
}

int32_t app_voice_assist_fir_lms_close(void)
{
    TRACE(0, "[%s] oneshot adapt anc close stream", __func__);
#ifdef FIR_LMS_DUMP
    audio_dump_deinit();
#endif
    return 0;
}



static int32_t _voice_assist_fir_lms_callback(void * buf, uint32_t len, void *other)
{

    if(stop_flag == 0){
        float ** input_data = buf;
        float * ff_data = input_data[0];  // error
        float * fb_data = input_data[1];  // error
        if(anc_ff_fir_lms_process(fir_st,ff_data,fb_data,120)){
            stop_flag = 1;
            app_voice_assist_fir_lms_close();
        }
    } else {
        return 0;
    }
    return 0;
    
} 




// /***************************************************************************
//  *
//  * Copyright 2015-2019 BES.
//  * All rights reserved. All unpublished rights reserved.
//  *
//  * No part of this work may be used or reproduced in any form or by any
//  * means, or stored in a database or retrieval system, without prior written
//  * permission of BES.
//  *
//  * Use of this work is governed by a license granted by BES.
//  * This work contains confidential and proprietary information of
//  * BES. which is protected by copyright, trade secret,
//  * trademark and other intellectual property rights.
//  *
//  ****************************************************************************/
// #include "hal_trace.h"
// #include "app_anc_assist.h"
// #include "app_voice_assist_fir_lms.h"
// #include "anc_select_mode.h"
// #include "anc_assist.h"

// #include "aud_section.h"
// #include "anc_process.h"
// #include "arm_math.h"
// #include "audio_dump.h"
// #include "app_utils.h"

// #define M_PI           3.14159265358979323846  /* pi */
// #define H_spk_fir_len (120)
// #define LOCAL_FIR_LEN (360)
// #define ANC_LMS_HEAP_SIZE (1024 * 10)

// #ifdef FIR_LMS_DUMP
// static short tmp_data[360];
// #endif
// // static float pre_coef_a[26] = {0};
// // static float pre_coef_b[26] = {0};



// ////anc_set_fir_cfg
// static int cache_cnt = 0;
// float H_spk[H_spk_fir_len] = {0.126522,0.105709,0.051654,0.004996,-0.031214,-0.005970,0.032631,0.047896,0.033072,0.005751,-0.007517,-0.005317,0.001709,-0.003056,-0.014122,-0.021449,-0.025251,-0.012018,-0.007822,-0.007517,-0.013460,-0.020721,-0.023408,-0.024553,-0.013408,-0.009074,-0.010512,-0.020759,-0.032143,-0.035957,-0.023852,-0.007429,-0.001547,-0.016533,-0.033757,-0.039387,-0.021721,-0.001780,0.003649,-0.006745,-0.012762,-0.008212,0.004123,0.018654,0.012448,0.001803,-0.011794,0.010390,0.010058,0.024278,0.020590,0.008258,-0.012467,-0.019324,-0.002558,0.033250,0.075784,0.061304,0.006301,-0.055613,-0.050016,0.016252,0.103407,0.139254,0.090256,-0.008843,-0.074095,-0.073918,-0.013752,0.036279,0.041944,0.006849,-0.034905,-0.053169,-0.037859,-0.011327,0.016886,0.023288,0.010380,-0.030532,-0.016515,-0.015538,0.001236,0.003142,-0.008594,-0.022072,-0.027407,-0.014593,-0.003433,-0.006940,-0.019894,-0.024176,-0.010297,0.006352,0.012380,0.006393,-0.007492,-0.009850,-0.004805,0.004903,0.014141,0.018989,0.019073,0.002617,0.000689,0.003300,0.009726,0.016558,0.014825,-0.001446,-0.014904,-0.021233,-0.019781,-0.011318,-0.004426,-0.002751,-0.002995,-0.000932,-0.001818,-0.005127,};

// static const int max_cnt = 100;
// static const int period_cnt = 5;
// static float error_energy = 0;
// static struct_anc_fir_cfg fir_cfg=
// {
//     .anc_fir_cfg_ff_l.fir_bypass_flag=0,
//     .anc_fir_cfg_ff_l.fir_len=500,
//     .anc_fir_cfg_ff_l.fir_coef[0]=32767*256,
// };
// static int stop_cnt = 0;

// static int32_t _voice_assist_fir_lms_callback(void *buf, uint32_t len, void *other);
// static enum FIR_LMS_STATE oneshot_adaptive_anc_state = FIR_LMS_STATE_RUNNING;

// int32_t app_voice_assist_fir_lms_init(void)
// {
//     oneshot_adaptive_anc_state = FIR_LMS_STATE_RUNNING;
//     app_anc_assist_register(ANC_ASSIST_USER_FIR_LMS, _voice_assist_fir_lms_callback);
//     TRACE(2,"##################################");
//     return 0;
// }

// static int running_flag = 0;
// int32_t app_voice_assist_fir_lms_reset(void){
//     fir_cfg.anc_fir_cfg_ff_l.fir_coef[0] =(int32_t)(1*32767*256);
//     for(int i = 1;i<fir_cfg.anc_fir_cfg_ff_l.fir_len;i++){
//         fir_cfg.anc_fir_cfg_ff_l.fir_coef[i] = 0; 
//     }
//     for(int i = 0 ;i<LOCAL_FIR_LEN;i++){
//         local_fir[i] = 0;
//     }
//     local_fir[0] = 1.0;
//     running_flag = 0;
//     TRACE(0," reset to old one %d %d",fir_cfg.anc_fir_cfg_ff_l.fir_coef[0],fir_cfg.anc_fir_cfg_ff_l.fir_coef[1]);
//     anc_set_fir_cfg(&fir_cfg,ANC_FEEDFORWARD);
//     return 0;
// }

// void set_fixed_fir_filter(void){
//     running_flag = 1;
//     for(int i = 0 ;i<LOCAL_FIR_LEN;i++){
//         fir_cfg.anc_fir_cfg_ff_l.fir_coef[i] = fixed_fir_filter[i];
//     }
//     TRACE(0," reset to new one %d %d",fir_cfg.anc_fir_cfg_ff_l.fir_coef[0],fir_cfg.anc_fir_cfg_ff_l.fir_coef[1]);
//     anc_set_fir_cfg(&fir_cfg,ANC_FEEDFORWARD);
//     for(int i =0;i<LOCAL_FIR_LEN;i++){        
//         fir_delta[i] = 0;
//     }
//     error_energy = 0;
//     cache_cnt = 0;
// }


// int32_t app_voice_assist_fir_lms_open(void)
// {
//     TRACE(0, "[%s] oneshot adapt anc start stream", __func__);

//     app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
//     stop_cnt = 0;
//     local_fir[0] = 1;

// // init iir filter
//     fir_cfg.anc_fir_cfg_ff_l.fir_coef[0] =(int32_t)(1.0*32767*256);
//     fixed_fir_filter[0] = fir_cfg.anc_fir_cfg_ff_l.fir_coef[0];
//     for(int i = 1;i<fir_cfg.anc_fir_cfg_ff_l.fir_len;i++){
//         fir_cfg.anc_fir_cfg_ff_l.fir_coef[i] = 0; 
//         fixed_fir_filter[i] = fir_cfg.anc_fir_cfg_ff_l.fir_coef[i];
//     }


//     for(int i = 1 ;i<LOCAL_FIR_LEN;i++){
//         local_fir[i] = 0.0;
//     }
//     local_fir[0] = 1.0;
    
//     anc_set_fir_cfg(&fir_cfg,ANC_FEEDFORWARD);
//     app_anc_assist_open(ANC_ASSIST_USER_FIR_LMS);
// #ifdef FIR_LMS_DUMP
//     audio_dump_init(120,sizeof(short),2);
// #endif
//     return 0;
// }

// int32_t app_voice_assist_fir_lms_close(void)
// {
//     oneshot_adaptive_anc_state = FIR_LMS_STATE_STOP;
//     TRACE(0, "[%s] oneshot adapt anc close stream", __func__);
// #ifdef FIR_LMS_DUMP
//     audio_dump_deinit();
// #endif
//     return 0;
// }


// static void gradient_descent(float* input, float error){
//     for(int i=0;i<LOCAL_FIR_LEN;i++){
//         fir_delta[i] +=   error * input[LOCAL_FIR_LEN-i] / 32768 / 32768 ;
//     }
// }

// static void add_h_spk(float * signal,int len,float * out){
//     // FIR
//     for(int i = 0; i < len ; i++) {
//         // out[i] = signal[i];
//         for(int j = 0; j< H_spk_fir_len-1; j++){
//             H_spk_cache[j] = H_spk_cache[j+1];
//         }
//         H_spk_cache[H_spk_fir_len-1] = signal[i];
//         out[i] = 0;
//         for(int j = 0; j < H_spk_fir_len; j++ ) {
//             out[i] += H_spk_cache[H_spk_fir_len-1-j] * H_spk[j];
//         }
//     }
// }

// static void bandpass_filter(float * input,int len){
//     // const float NUM[5] = {0.01185768,  0.        , -0.02371537,  0.        ,  0.01185768}; //400-1000 / 8000
//     // const float DEN[5] = {1.        , -3.55609299,  4.849508  , -3.00683133,  0.71663387};

//     const float NUM[5] = {0.02008337,  0.        , -0.04016673,  0.        ,  0.02008337}; //200-1000 / 8000
//     const float DEN[5] = {1.        , -3.50578894,  4.66482454, -2.79961684,  0.64135154};

//     // const float NUM[5] = {0.02482791,  0.        , -0.04965581,  0.        ,  0.02482791}; //100-1000 / 8000
//     // const float DEN[5] = {1.        , -3.48015288,  4.57337369, -2.69979125,  0.60675903};

//     static float y0 = 0, y1 = 0, y2 = 0, y3 = 0 , y4 = 0 , x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    
//     for(int i=0; i < len;i++ ){
//         x0 = input[i];
//         y0 = x0*NUM[0] + x1*NUM[1] + x2*NUM[2] + x3 * NUM[3] + x4*NUM[4] - y1*DEN[1] - y2*DEN[2]- y3*DEN[3]- y4*DEN[4];
//         y4 = y3;
//         y3 = y2;
//         y2 = y1;
//         y1 = y0;
//         x4 = x3;
//         x3 = x2;
//         x2 = x1;
//         x1 = x0;
//         input[i] = y0;
//     }
// }

// static void bandpass_filter2(float * input,int len){
//     // const float NUM[5] = {0.01185768,  0.        , -0.02371537,  0.        ,  0.01185768}; //400-1000 / 8000
//     // const float DEN[5] = {1.        , -3.55609299,  4.849508  , -3.00683133,  0.71663387};

//     const float NUM[5] = {0.02008337,  0.        , -0.04016673,  0.        ,  0.02008337}; //200-1000 / 8000
//     const float DEN[5] = {1.        , -3.50578894,  4.66482454, -2.79961684,  0.64135154};

//     // const float NUM[5] = {0.02482791,  0.        , -0.04965581,  0.        ,  0.02482791}; //100-1000 / 8000
//     // const float DEN[5] = {1.        , -3.48015288,  4.57337369, -2.69979125,  0.60675903};

//     static float y0 = 0, y1 = 0, y2 = 0, y3 = 0 , y4 = 0 , x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    
//     for(int i=0; i < len;i++ ){
//         x0 = input[i];
//         y0 = x0*NUM[0] + x1*NUM[1] + x2*NUM[2] + x3 * NUM[3] + x4*NUM[4] - y1*DEN[1] - y2*DEN[2]- y3*DEN[3]- y4*DEN[4];
//         y4 = y3;
//         y3 = y2;
//         y2 = y1;
//         y1 = y0;
//         x4 = x3;
//         x3 = x2;
//         x2 = x1;
//         x1 = x0;
//         input[i] = y0;
//     }
// }

// static void upsample_filter(float * input, float* output,int len){

//     const float NUM[5] = { 5.12320597e-06, 2.04928239e-05, 3.07392358e-05, 2.04928239e-05,
//         5.12320597e-06}; //  6000 / 192000
//     const float DEN[5] = {  1.        , -3.74350676,  5.2629038 , -3.29294328,  0.77362822};


//     static float y0 = 0, y1 = 0, y2 = 0, y3 = 0 , y4 = 0 , x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    
//     for(int i=0; i < len;i++ ){
//         x0 = input[i];
//         for(int j = 0; j < 24;j++) {
//             y0 = x0*NUM[0] + x1*NUM[1] + x2*NUM[2] + x3 * NUM[3] + x4*NUM[4] - y1*DEN[1] - y2*DEN[2]- y3*DEN[3]- y4*DEN[4];
//             y4 = y3;
//             y3 = y2;
//             y2 = y1;
//             y1 = y0;
//             x4 = x3;
//             x3 = x2;
//             x2 = x1;
//             x1 = x0;
//             output[24*i+j] = y0;
//         }
//     }
// }

// static void upsample_filter2(float * input, float* output,int len){

//     const float NUM[5] = { 5.12320597e-06, 2.04928239e-05, 3.07392358e-05, 2.04928239e-05,
//         5.12320597e-06}; //  6000 / 192000
//     const float DEN[5] = {  1.        , -3.74350676,  5.2629038 , -3.29294328,  0.77362822};


//     static float y0 = 0, y1 = 0, y2 = 0, y3 = 0 , y4 = 0 , x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    
//     for(int i=0; i < len;i++ ){
//         x0 = input[i];
//         for(int j = 0; j < 24;j++) {
//             y0 = x0*NUM[0] + x1*NUM[1] + x2*NUM[2] + x3 * NUM[3] + x4*NUM[4] - y1*DEN[1] - y2*DEN[2]- y3*DEN[3]- y4*DEN[4];
//             y4 = y3;
//             y3 = y2;
//             y2 = y1;
//             y1 = y0;
//             x4 = x3;
//             x3 = x2;
//             x2 = x1;
//             x1 = x0;
//             output[24*i+j] = y0;
//         }
//     }
// }


// static void fir_transfer(void){
//     for(int i = 0; i < LOCAL_FIR_LEN ;i++ ){
//         fir_cfg.anc_fir_cfg_ff_l.fir_coef[i] = (int32_t)(local_fir[i] * 32767*256 );
//     }
// }
// static void copy_fix_fir_filter(void){
//     for(int i = 0;i<LOCAL_FIR_LEN;i++){
//         fixed_fir_filter[i] = fir_cfg.anc_fir_cfg_ff_l.fir_coef[i];
//     }
// }



// static float inter_result[120*24] ={0};
// static float inter_result1[120*24] ={0};
// static float ref_spk[120] ={0};
// static float mem0[6] = {0};
// static float mem1[6] = {0};
// static float mem2[6] = {0};
// static float mem3[6] = {0};
// static float mem4[6] = {0};
// static float biquad_tmp1[360] = {0};
// static float biquad_tmp2[360] = {0};


// static float fix_coef_0_b[3] =  {-1.390648 , 2.752544 , -1.362366};
// static float fix_coef_0_a[3] =  {1.000000 , -1.994199 , 0.994226};
// static float fix_coef_1_b[3] =  {1.151443 , -1.834991 , 0.746954};
// static float fix_coef_1_a[3] =  {1.000000 , -1.834991 , 0.898397};
// static float fix_coef_2_b[3] =  {0.396111 , -0.532297 , 0.356670};
// static float fix_coef_2_a[3] =  {1.000000 , -0.532297 , -0.247219};
// static float fix_coef_3_b[3] =  {0.421697 , 0.235499 , 0.157253};
// static float fix_coef_3_a[3] =  {1.000000 , -0.558456 , 0.372905};
// static float fix_coef_4_b[3] =  {0.956484 , -1.456808 , 0.620354};
// static float fix_coef_4_a[3] =  {1.000000 , -1.456808 , 0.576838};



// static void biquad_process(float * input, float * output,int len,int skip_size, float * bi_coef_a, float * bi_coef_b, float* mem){
//     // mem [input-1,input-2,output-1,output-2]
//     for(int i = 0; i < len; i++){
//         output[i] = 0;
//     }
//     float out_0 = mem[0];
//     float out_1 = mem[1];
//     float in_0 = mem[2];
//     float in_1 = mem[3];
//     output[0] = input[0] * bi_coef_b[0] + in_0 * bi_coef_b[1] + in_1 * bi_coef_b[2] - out_0 * bi_coef_a[1] - out_1 * bi_coef_a[2];
//     output[1] = input[1] * bi_coef_b[0] + input[0] * bi_coef_b[1] + in_0 * bi_coef_b[2] - output[0] * bi_coef_a[1] - out_0 * bi_coef_a[2];

//     for(int i = skip_size; i < len; i++){
//         output[i] = input[i] * bi_coef_b[0] + input[i-1] * bi_coef_b[1] + input[i-2] * bi_coef_b[2] - output[i-1] * bi_coef_a[1] - output[i-2] * bi_coef_a[2];
//     }

//     mem[2] = input[len-1];
//     mem[3] = input[len-2];
//     mem[0] = output[len-1];
//     mem[1] = output[len-2];

// }




// static int32_t _voice_assist_fir_lms_callback(void * buf, uint32_t len, void *other)
// {
//     if(stop_cnt == max_cnt){
//         stop_cnt++;
//         app_anc_assist_close(ANC_ASSIST_USER_FIR_LMS);
//         return 0;
//     } else if(stop_cnt > max_cnt){
//         return 0;
//     }
//     float ** input_data = buf;
//     float * ff_data = input_data[0];  // error
//     float * fb_data = input_data[1];  // error
//     // float * ref_data = input_data[2]; // ref data
//     len = 120;

//     if(1){
//         add_h_spk(ff_data,len,biquad_tmp2); // add H_spk to ref
//         biquad_process(biquad_tmp2,biquad_tmp1,120,2,fix_coef_0_a,fix_coef_0_b,mem0);
//         biquad_process(biquad_tmp1,biquad_tmp2,120,2,fix_coef_1_a,fix_coef_1_b,mem1);
//         biquad_process(biquad_tmp2,biquad_tmp1,120,2,fix_coef_2_a,fix_coef_2_b,mem2);
//         biquad_process(biquad_tmp1,biquad_tmp2,120,2,fix_coef_3_a,fix_coef_3_b,mem3);
//         biquad_process(biquad_tmp2,ref_spk,120,2,fix_coef_4_a,fix_coef_4_b,mem4);
//     } else {
//         add_h_spk(ff_data,len,ref_spk); // add H_spk to ref 
//     }
    
//     bandpass_filter(ref_spk,len); // bandpass 200-1000, can change
//     bandpass_filter2(fb_data,len);

// #ifdef FIR_LMS_DUMP
//     audio_dump_clear_up();
//     for(int i = 0; i< 120; i++) {
//         tmp_data[i] = (short)(biquad_tmp2[i]/256);
//     }
//     audio_dump_add_channel_data(0,tmp_data,120);
//     for(int i = 0; i< 120; i++) {
//         tmp_data[i] = (short)(ref_spk[i]/256);
//     }
//     audio_dump_add_channel_data(1,tmp_data,120); 
//     audio_dump_run();
// #endif  

//     // upsample to 384k
//     upsample_filter(ref_spk,inter_result,len);
//     upsample_filter2(fb_data,inter_result1,len);
//     len *= 24;



//     if(1){ //running_flag == 1
//         if(cache_cnt< period_cnt){
//             cache_cnt++;
//             for(int i = H_spk_fir_len ;i < H_spk_fir_len+50 ; i++ ){
//                 gradient_descent(&inter_result[i],inter_result1[i+LOCAL_FIR_LEN]);
//             }
//             for(int i =0;i<120;i++){
//                 error_energy += fb_data[i] * fb_data[i];
//             }

//         } else if(cache_cnt == period_cnt) {
//             // update fir filter
//             float scale_size = 1;
//             error_energy /= (float)period_cnt;
//             float tmp_energy = error_energy;
//             float gap_size = 1000;
//             // int gap_cnt = 0;
//             while(tmp_energy > gap_size){
//                 scale_size *= 0.1;
//                 tmp_energy /= gap_size;
//             }
//             float mean_error = 0;
//             for(int i =0;i<LOCAL_FIR_LEN;i++){
//                 fir_delta[i] = fir_delta[i] / 50.0f / (float)period_cnt;
//                 if(fir_delta[i] < 0){
//                     mean_error -= fir_delta[i];
//                 } else {
//                     mean_error += fir_delta[i];
//                 }
//                 local_fir[i] += scale_size*fir_delta[i];
//                 if(local_fir[i] >= 1){
//                     local_fir[i] = 1;
//                 }
//                 if(local_fir[i] <= -1){
//                     local_fir[i] = -1;
//                 }
            
                
//                 fir_delta[i] = 0;
//             }
            
//             mean_error /= (float)LOCAL_FIR_LEN;
//             fir_transfer();
//             TRACE(2,"%d %d %d %d",(int)(1000*local_fir[0]),(int)(100000*mean_error),(int)(100000*scale_size),(int)(error_energy/1000));
//             error_energy = 0;
//             anc_set_fir_cfg(&fir_cfg,ANC_FEEDFORWARD);
//             copy_fix_fir_filter();
//             cache_cnt++;
//         } else {
//             cache_cnt++;
//             if(cache_cnt >= 2*period_cnt){
//                 stop_cnt++;
//                 cache_cnt = 0;
//             }
            
//         }
//     }
    
    
    




//     return 0;
// } 
