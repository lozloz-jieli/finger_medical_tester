/*
* Copyright (c) HeartVoice Medical Technology Co., Ltd. 2023-2024. All rights reserved.
* Description: �ĵ�����㷨ͷ�ļ�
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
��ʼ������
˵����ÿ�βɼ�ǰִ�г�ʼ��
*/
extern void EcgArrhyAnalyInit(void);

/*
������
˵����0.2sִ��һ�Σ�ECG����125hz������int������25
*/
extern uint16_t EcgArrhyAnaly(const EcgDataBuffer* ecgDataBuffer);


#endif