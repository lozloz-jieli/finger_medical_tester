#ifndef __HEART_ADC_H__
#define __HEART_ADC_H__





void collect_heart_sound_handle(void);
void haert_adc_printf(void);
void clear_axis_status(void);


typedef struct __HEART_VAR
{
    int cycle_timer_id;
    char drop_flag;
    int x_flag;                 //画心跳曲线点的X轴
}HEART_VAR;


#define COLLECT_STEP        5

extern HEART_VAR heart_var;


#endif












