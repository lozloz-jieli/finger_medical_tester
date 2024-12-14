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
#include "extern_charge.h"

#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[EX_CHARGE]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

extern void sys_key_event_disable();
extern void heart_poweroff(void);



/*
**********************************************************************
FUNCTION：满电检测io口初始化
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/
void ex_charge_in_detect_init(void)
{
    gpio_set_die(TCFG_VBAT_5V_WAKE,1);
    gpio_set_dieh(TCFG_VBAT_5V_WAKE,1);
    gpio_set_pull_down(TCFG_VBAT_5V_WAKE, 1);
    gpio_set_pull_up(TCFG_VBAT_5V_WAKE, 0);
    gpio_set_direction(TCFG_VBAT_5V_WAKE, 1);    
}

/*
**********************************************************************
FUNCTION：满电检测io口关闭使能
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/

void ex_charge_in_detect_disable(void)
{
    gpio_set_die(TCFG_VBAT_5V_WAKE,0);
    gpio_set_dieh(TCFG_VBAT_5V_WAKE,0);
    gpio_set_pull_down(TCFG_VBAT_5V_WAKE, 0);
    gpio_set_pull_up(TCFG_VBAT_5V_WAKE, 0);
    gpio_set_direction(TCFG_VBAT_5V_WAKE, 1);    
}



/*
**********************************************************************
FUNCTION：满电检测io口初始化
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/
void ex_charge_full_detect_init(void)
{
    gpio_set_die(TCFG_FULL_PORT,1);
    gpio_set_dieh(TCFG_FULL_PORT,1);
    gpio_set_pull_down(TCFG_FULL_PORT, 0);
    gpio_set_pull_up(TCFG_FULL_PORT, 1);
    gpio_set_direction(TCFG_FULL_PORT, 1);    
}

/*
**********************************************************************
FUNCTION：满电检测io口关闭使能
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/

void ex_charge_full_detect_disable(void)
{
    gpio_set_die(TCFG_FULL_PORT,0);
    gpio_set_dieh(TCFG_FULL_PORT,0);
    gpio_set_pull_down(TCFG_FULL_PORT, 0);
    gpio_set_pull_up(TCFG_FULL_PORT, 0);
    gpio_set_direction(TCFG_FULL_PORT, 1);    
}

/*
**********************************************************************
FUNCTION：满电检测检测timer
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/
u16 full_vbat_timer_id;

void enter_charge_close_device(void)
{
    app_var.charge_flag = 1;
    auto_power_off_delete();
    sys_key_event_disable();
}

void deal_function_in_charge(void)
{
    collect_timer_cnt_delete();           //关闭计时定时器
    flash_one_second_delete();             //关闭显示秒和心跳的定时器
    enter_charge_close_device();         //关闭充电状态一下配置状态
    clear_screen();
}

void full_vbat_detect_deal(void)
{
    static u8 stauts_flag;

    app_var.ldo_in_ret = gpio_read(TCFG_VBAT_5V_WAKE);
    app_var.full_ret = gpio_read(TCFG_FULL_PORT);
    // g_printf("TCFG_VBAT_5V_WAKE = %d,TCFG_FULL_PORT = %d",app_var.ldo_in_ret,app_var.full_ret);


#if 1
    if(gpio_read(TCFG_VBAT_5V_WAKE) == 0 && gpio_read(TCFG_FULL_PORT) == 1 && stauts_flag!=1 )
    {
        y_printf("not ccharge");
        stauts_flag = 1;
        if(app_var.charge_flag == 1){
            heart_poweroff();                                          //关机
        }
    }
    else if(gpio_read(TCFG_VBAT_5V_WAKE) == 1 &&  gpio_read(TCFG_FULL_PORT) == 0 && stauts_flag!=2)
    {
        y_printf("in charge and not full");
        stauts_flag = 2;
        deal_function_in_charge();
        charge_mode();
    }
    else if(gpio_read(TCFG_VBAT_5V_WAKE) == 1 &&  gpio_read(TCFG_FULL_PORT) == 1 && stauts_flag!=3)
    {
        y_printf("in charge and full");
        stauts_flag = 3;
        deal_function_in_charge();
        charg_full_mode();
    }
#endif

}

void full_vbat_detect_handle(void)
{
    ex_charge_full_detect_init();
    ex_charge_in_detect_init();
    if(full_vbat_timer_id == 0){
        full_vbat_timer_id = sys_timer_add(NULL,full_vbat_detect_deal,500);
    }
}



/*
**********************************************************************
FUNCTION：振动开关打印函数
PARAMETER：none
RETURN：None
MEMORY：
AUTOR：lozloz
DATA：2024-9-10
VERSION：V0.0
**********************************************************************
*/

void ex_charge_printf(void)
{
    log_info("%s",__func__);
}



