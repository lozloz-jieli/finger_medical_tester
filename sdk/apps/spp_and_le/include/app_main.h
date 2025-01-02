#ifndef APP_MAIN_H
#define APP_MAIN_H

typedef struct _APP_VAR {
    s8 music_volume;
    s8 call_volume;
    s8 wtone_volume;
    u8 opid_play_vol_sync;
    u8 aec_dac_gain;
    u8 aec_mic_gain;
    u8 rf_power;
    u8 goto_poweroff_flag;
    u8 goto_poweroff_cnt;
    u8 play_poweron_tone;
    u8 remote_dev_company;
    u8 siri_stu;
    int auto_stop_page_scan_timer;     //用于1拖2时，有一台连接上后，超过三分钟自动关闭Page Scan
    volatile int auto_shut_down_timer;
    volatile int wait_exit_timer;
    u16 auto_off_time;
    u16 warning_tone_v;
    u16 poweroff_tone_v;
    u32 start_time;
    s8  usb_mic_gain;
    u8 ldo_in_ret;
    u8 full_ret;
    u8 charge_flag;
    u16 g_auto_shutdown_timer;
    u8 heart_status;                   //心跳 画点的状态  1：开始     0 ：停止
    u8 disp_collect_ok;                //显示30s采集ok的标志
    u8 disp_collect_fail;                //显示30s采集失败的标志
    u8 buzzer_status;
    u8 buzzer_cnt;
    u8 buzz_mute;                   //蜂鸣器的mute住是否

    u16 set_ble_name_len;    //蓝牙名长度

    u16 Number_frames;       //记录帧数
    u8 flag_change_ble_name;        //改蓝牙名后置一  
    u8 reset_factory;         //是否修改了蓝牙名
    u8 bt_conn;             //蓝牙连接标志位
    u8 real_flag;           //实时数据上发标志位；
    u8 power_off_flag;         //关机标志位

} APP_VAR;

enum {
    KEY_USER_DEAL_POST = 0,
    KEY_USER_DEAL_POST_MSG,
    KEY_USER_DEAL_POST_EVENT,
    KEY_USER_DEAL_POST_2,
};

typedef struct __ELEC_HEART
{
    u8 power_second;
    u16 heart_second;                   //心跳的计时
    u16 heart_data;  
    u16 last_heart_data;                //记录心跳数据上下次的变化是否
    u8 low_power;
    u8 power_off_flag;                  //关机标志位
    u8 history_flag;                    //为了处理每份文件的云服务上传，好了一条文件上传一次
}ELEC_HEART;



extern APP_VAR app_var;
extern ELEC_HEART elec_heart;

void app_switch(const char *name, int action);

#endif
