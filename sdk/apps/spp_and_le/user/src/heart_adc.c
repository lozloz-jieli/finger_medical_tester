#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "3th_profile_api.h"
#include "le_common.h"
#include "rcsp_bluetooth.h"
#include "JL_rcsp_api.h"
#include "custom_cfg.h"
#include "btstack/btstack_event.h"
#include "gatt_common/le_gatt_common.h"
#include "app_main.h"
#include "heart_adc.h"
#include "ECG_hrs.h"
#include "math_deal.h"



#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[HEART_ADC]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

HEART_VAR heart_var;
EcgDataBuffer ecg_var;
EcgDataBuffer ecg_var_vol;
AXIS_XY axis_xy;

extern void Get_ECG_ad_value_deal(u16 adv_value);
extern void history_data_write_deal(u16 ecg_vol);
extern u32 adc_sample_ch_one(u32 gpio, u32 ch);
extern u32 adc_fast_sample(u32 ch); //96M频率耗时1us-2us之间
extern u32 adc_get_voltage_special(u32 ch);


void ECG_init(void)
{
    gpio_set_direction(TCFG_ECG_MCU_OUT_PIN, 0);
    gpio_set_pull_down(TCFG_ECG_MCU_OUT_PIN, 0);
    gpio_set_pull_up(TCFG_ECG_MCU_OUT_PIN, 0);        
    gpio_set_output_value(TCFG_ECG_MCU_OUT_PIN,1);    //输出电平，为ECG电路供电
}     



void adc_heart_detect_init(void)
{
    log_info("%s",__func__);

    adc_add_sample_ch(TCFG_ADC_DETECTION_HEART_CH);          //注意：初始化AD_KEY之前，先初始化ADC

    gpio_set_die(TCFG_ADC_DETECTION_HEART, 0);
    gpio_set_dieh(TCFG_ADC_DETECTION_HEART, 0);
    gpio_set_direction(TCFG_ADC_DETECTION_HEART, 1);
    gpio_set_pull_down(TCFG_ADC_DETECTION_HEART, 0);
    gpio_set_pull_up(TCFG_ADC_DETECTION_HEART, 0);

}

void notify_draw_line(void)
{
    struct sys_event e;
    e.type = SYS_KEY_EVENT;
    e.u.key.init = 1;
    e.u.key.type = 0;//区分按键类型
    e.u.key.event = 0;
    e.u.key.value = 0;

    e.arg  = (void *)DEVICE_EVENT_FROM_DRAW;
    sys_event_notify(&e);

}




void clear_axis_status(void)
{
    memset(&axis_xy,0,sizeof(AXIS_XY));
    memset(&ecg_var,0,sizeof(EcgDataBuffer));
    memset(&ecg_var_vol,0,sizeof(EcgDataBuffer));
}


void limit_y_deal(void)
{
    if(axis_xy.y_last>60){
        axis_xy.y_last = 60;
    }
    else if(axis_xy.y_last < 2){
        axis_xy.y_last = 1;
    }
}


void deal_adc_value(int first_adc,int tem_adc)
{
#if 1     //开始做心电曲线处理
        heart_var.x_flag;
        if(ecg_var.ecgDataLen % COLLECT_STEP == 0){
            //打点到坐标
            heart_var.x_flag++;


#if 1   //阈值处理
            if(tem_adc > LIMIT_HIGH)
            {
                tem_adc = LIMIT_HIGH;
            }
            else if(tem_adc < BASE_NUM)
            {
                tem_adc = BASE_NUM;
            }
#endif

            if(heart_var.x_flag == 1){                                                           //X轴为1的时候                                   
                axis_xy.x_last = (heart_var.x_flag-1)*COLLECT_X_MUL+1;
#if 1   //阈值处理
                if(ecg_var.ecgDataArr[ecg_var.ecgDataLen-COLLECT_STEP] > LIMIT_HIGH){
                    first_adc = ecg_var.ecgDataArr[ecg_var.ecgDataLen-COLLECT_STEP];
                    first_adc = LIMIT_HIGH;
                }
                else if(tem_adc < BASE_NUM)
                {
                    first_adc = ecg_var.ecgDataArr[ecg_var.ecgDataLen-COLLECT_STEP];
                    first_adc = BASE_NUM;
                }else
                {
                    first_adc = ecg_var.ecgDataArr[ecg_var.ecgDataLen-COLLECT_STEP];
                }
#endif                
                axis_xy.y_last =  first_adc - BASE_NUM;
            }else{
                axis_xy.x_last = axis_xy.x_cur;
                axis_xy.y_last = axis_xy.y_cur;
            }
            axis_xy.x_cur = heart_var.x_flag*COLLECT_X_MUL+1;    //X坐标点  x->位移两格x+2
            axis_xy.y_cur = tem_adc - BASE_NUM;      //Y心跳

#if 0            
            //先清除要显示的
            clear_list_heart_block(axis_xy.x_last,axis_xy.x_cur,COLOR_BLACK);
            //显示对应的折线
            LCD_DrawLine(axis_xy.x_last,axis_xy.y_last,axis_xy.x_cur,axis_xy.y_cur,WHITE);
#else
            limit_y_deal();
            notify_draw_line();
#endif

#if 0
            r_printf("x_last = %d,y_last = %d,x_cur = %d,y_cur = %d",axis_xy.x_last,axis_xy.y_last,axis_xy.x_cur,axis_xy.y_cur);
#endif

            if( axis_xy.x_cur > 158){
                heart_var.x_flag = 0;
            }

        }
#endif
}


void collect_heart_sound_deal(void)
{
    if(elec_heart.low_power == 1){          // battery <3.3v
        // putchar('l');
        return;
    }
    
    if(elec_heart.power_off_flag == 1){         //power_off
        // putchar('f');
        return; 
    }
    
    if(!elec_heart.heart_second){           //心跳计时<0,不做采集
        // putchar('X');
        return;
    }

    if(app_var.disp_collect_all_ok == 1){   //手指脱落时候，大于30s采集成功，暂停一段时间
        return;
    }

    if(get_collect_ok_status()){             //30s采集成功，暂停一段时间
        return;
    }

    if(loz_exflash_var.history_flag == 1){
        return;                             //正在上传历史数据，等上传完后者等蓝牙断卡或者停止才能继续下一步；
    }

    int adc_value;
    int adc_vol;
    int tem_adc;
    int first_adc;
    int heart_data;
    // adc_value = adc_get_value(TCFG_ADC_DETECTION_HEART_CH);
    // adc_value = adc_get_voltage(TCFG_ADC_DETECTION_HEART_CH);
    // adc_value = adc_sample_ch_one(c,TCFG_ADC_DETECTION_HEART_CH);              //快速获取adc 电压
    adc_value = adc_fast_sample(TCFG_ADC_DETECTION_HEART_CH);
    // adc_value = adc_get_voltage_special(TCFG_ADC_DETECTION_HEART_CH);
    // adc_value = adc_value/VOLTAGE_MODE_DIV;
    adc_value = adc_value*VOLTAGE_MODE_MUL;
    tem_adc = adc_value;
    // y_printf("adc_value = %d",adc_value);
    // printf("%d",adc_value);
    // printf("%d",adc_vol);


/*************************************************app_发送数据************************************************/
    Get_ECG_ad_value_deal(adc_value);
/*************************************************历史数据存储************************************************/
    history_data_write_deal(adc_value);

    // if(get_collect_ok_status()){             //采集成功，暂停画图一段时间
    //     return;
    // }

#if 1
    if(app_var.heart_status){
        ecg_var.ecgDataArr[ecg_var.ecgDataLen] = adc_value;                     //adc采集
        ecg_var.ecgDataLen++;
        ecg_var_vol.ecgDataArr[ecg_var_vol.ecgDataLen] = adc_vol;              //adc电压采集
        ecg_var_vol.ecgDataLen++;

        // printf("%d",adc_value);        
/*************************************************开始画曲线************************************************/
        deal_adc_value(first_adc,tem_adc);


    }else{
        putchar('N');
    }
#endif

    if(ecg_var.ecgDataLen == 25){
        heart_data = EcgArrhyAnaly(&ecg_var);
        // heart_data = EcgArrhyAnaly(&ecg_var_vol);
        elec_heart.heart_data = heart_data;
        // put_buf(&ecg_var.ecgDataArr,ecg_var.ecgDataLen*4);
        // y_printf("heart_data = %d",heart_data);
        ecg_var.ecgDataLen = 0;
        ecg_var_vol.ecgDataLen = 0;
    }
}


void collect_heart_sound_handle(void)
{
    log_info("%s",__func__);
    ECG_init();
    // ECG_drop_port_init();
    EcgArrhyAnalyInit();
    adc_heart_detect_init();
    if(heart_var.cycle_timer_id == 0){
        heart_var.cycle_timer_id = sys_s_hi_timer_add(NULL,collect_heart_sound_deal,8);
        // heart_var.cycle_timer_id = sys_timer_add(NULL,collect_heart_sound_deal,8);
    }
}


void haert_adc_printf(void)
{
    log_info("%s",__func__);
}

