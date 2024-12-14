/*
* Copyright (c) HeartVoice Medical Technology Co., Ltd. 2023-2024. All rights reserved.
* Description: 心电分析算法头文件
* Author: K.K
* Create time: 2024/7/15
* version: V1.0.4
*/


#ifndef ECG_ALA_H
#define ECG_ALA_H


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif




#include<stdint.h>

#define ECG_DATA_MAX_LEN 25 /* 200ms for 125hz */



typedef struct {
    uint8_t ecgDataLen;
    int ecgDataArr[ECG_DATA_MAX_LEN];
} EcgDataBuffer;



/*
初始化函数
说明：每次采集前执行初始化
*/
extern void EcgArrhyAnalyInit(void);

/*
主函数
说明：0.2s执行一次；ECG数据125hz，类型int，长度25
*/
extern uint16_t EcgArrhyAnaly(const EcgDataBuffer* ecgDataBuffer);


#endif