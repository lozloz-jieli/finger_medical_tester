#include "app_config.h"
#include "system/includes.h"
#include "asm/includes.h"
#include "device/key_driver.h"
#include "asm/power/p33.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "btstack/bluetooth.h"
#include "lost.h"
#include "app_power_manage.h"
#include "asm/mcpwm.h"
#include "app_main.h"



#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[APP_UI]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

extern void mcu_send_control_app(u8 *buffer,u16 len);
extern void mcu_send_history_app(u8 *buffer,u16 len);

extern u32 exflash_offse;
extern u32 temp_exflash_offse;

enum{
    APP_HEAD,
    APP_CMD,
};

enum{
    APP_SYC_TIME = 0x01,                                   //同步时间戳
    APP_SYC_HIS_DATA,                               //同步历史数据
    APP_CHECK_HIS_DATA,                             //查询历史数据文件
    APP_SURE_HIS_DATA_OK,                            //确认一组数据同步完成
    APP_NOTIFY_HIS_DATA,                            //固件上行数据格式
    APP_START_REAL_DATA,                            //APP下行通知       开始实时数据测量
    APP_STOP_REAL_DATA,                            //APP下行通知        暂停实时数据测量
};


enum{
    UNIX_TIME1_H = 2,
    UNIX_TIME1_L,
    UNIX_TIME0_H,
    UNIX_TIME0_L,
};


const char China_Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void set_apprtc_time(u8* buffer,u16 len)
{
    struct sys_time app_data_def;
    T_Time app_time;
    u32 unix_time = (buffer[UNIX_TIME1_H] << 24) | (buffer[UNIX_TIME1_L] << 16) | (buffer[UNIX_TIME0_H] << 8) | buffer[UNIX_TIME0_L];
    r_printf("decimal %d, HEx 0x%x",unix_time,unix_time);
    localtime(unix_time,&app_time);

    // app_time.year += 2000;
    // app_time.month += 1;
    app_data_def.year = 2000+app_time.year;
    app_data_def.month = app_time.month + 1;
    app_data_def.day = app_time.day;
    app_data_def.hour =  app_time.hour + 8;         //time is USA,not CHINA,USA+8 = CHINA,
    if(app_data_def.hour>24){
        app_data_def.hour -= 24;
        app_data_def.day +=1;
        //not leap year,don'think  leap 2month
        if(app_data_def.day>China_Days[app_data_def.month-1]){
            app_data_def.month += 1;
            if(app_data_def.month > 12){
                app_data_def.month = 1;
                app_data_def.year += 1;    //new year;
            }
        }
    }
    app_data_def.min = app_time.minute;
    app_data_def.sec = app_time.second;

    g_printf("%d-%d-%d %d:%d:%d\n", app_time.year, app_time.month, app_time.day,
            app_time.hour, app_time.minute, app_time.second);

    g_printf("%d-%d-%d %d:%d:%d\n", app_data_def.year, app_data_def.month, app_data_def.day,
            app_data_def.hour, app_data_def.min, app_data_def.sec);

    rtc_ioctl(IOCTL_SET_SYS_TIME, (u32)&app_data_def);


}

/*
**********************************************************************
函数功能：应答对应的文件传输完成
函数形参：
函数返回值：None
备注：
日期：2024年12月10日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void ack_file_num_ok(u8 file)
{
    u8 ack_file_buffer[5] = {0x5a,0x04,0x05,0x00,0x00};
    ack_file_buffer[4] = file;
    mcu_send_control_app(ack_file_buffer,sizeof(ack_file_buffer));
}



void app_write_cmd_control(u8 *data,u16 len)
{
    if(data[APP_HEAD] == 0xA5){

    }else{
        return;
    }

    switch (data[APP_CMD])
    {
    case APP_SYC_TIME:
        set_apprtc_time(data,len);
        data[0] = 0x5A;
        mcu_send_control_app(data,len);
        break;
    
    case APP_SYC_HIS_DATA:
        //同步历史数据命令
        loz_exflash_var.history_flag = 1;           //让其他图形不能操作
        elec_heart.history_flag = 1;                //为了处理开始下一个文件操作的标志位

        clear_screen();
        syn_data_mode();
        history_data_read_handle();

        break;

    case APP_CHECK_HIS_DATA:

        u8 temp_buffer[5] = {0x5a,0x03,0x05,0x00,0x00};
        temp_buffer[4] = remain_history_file();
        mcu_send_control_app(temp_buffer,sizeof(temp_buffer));
        break;
        
    case APP_START_REAL_DATA:
        app_var.real_flag = 1;
        data[0] = 0x5A;
        mcu_send_control_app(data,len);        
        break;

    case APP_STOP_REAL_DATA:
        app_var.real_flag = 1;
        data[0] = 0x5A;
        mcu_send_control_app(data,len);
        break;
    default:
        break;
    }
}




void app_ui_printf(void)
{
    log_info("%s",__func__);
}
