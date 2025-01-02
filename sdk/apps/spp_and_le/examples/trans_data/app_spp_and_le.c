#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"
#include "os/os_api.h"
#include "btcontroller_config.h"
#include "btctrler/btctrler_task.h"
#include "config/config_transport.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_common.h"
#include "rcsp_bluetooth.h"
#include "rcsp_user_update.h"
#include "app_charge.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "app_comm_bt.h"
#include "usb/device/cdc.h"
#include "lcd.h"
#include "app_main.h"

#define LOG_TAG_CONST       SPP_AND_LE
#define LOG_TAG             "[SPP_AND_LE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_AUDIO_ENABLE
#include "tone_player.h"
#include "media/includes.h"
#include "key_event_deal.h"

extern void rtc_alarm_set_timer(u32 seconds);
extern void trans_disconnect(void);
extern void midi_paly_test(u32 key);
#endif/*TCFG_AUDIO_ENABLE*/


#if 1//CONFIG_APP_SPP_LE

static u8 is_app_spple_active = 0;
static u8 enter_btstack_num = 0;
extern void app_switch(const char *name, int action);
extern void DispColor(unsigned int color);
extern void DispColor_Dma(unsigned int color);
extern void X_timer_scan(void *priv);
extern void mcu_send_history_app(u8 *buffer,u16 len);

//---------------------------------------------------------------------
/*************************************************************************************************/
/*!
 *  \brief      进入软关机
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void spple_power_event_to_user(u8 event)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_POWER;
    e.u.dev.event = event;
    e.u.dev.value = 0;
    sys_event_notify(&e);
}

static void spple_set_soft_poweroff(void)
{
    r_printf("set_soft_poweroff\n");
    is_app_spple_active = 1;
    //必须先主动断开蓝牙链路,否则要等链路超时断开

#if TCFG_USER_BLE_ENABLE
    btstack_ble_exit(0);
#endif

#if 1
    ex_charge_full_detect_disable();
#endif

#if TCFG_USER_EDR_ENABLE
    btstack_edr_exit(0);
#endif

#if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
    //延时300ms，确保BT退出链路断开
    sys_timeout_add(NULL, power_set_soft_poweroff, WAIT_DISCONN_TIME_MS);
#else
    power_set_soft_poweroff();
#endif
}

extern AXIS_XY axis_xy;


void heart_poweroff(void)
{
    spple_set_soft_poweroff();
}

/* #define LED_GPIO_PIN     IO_PORTB_05//IO_PORTB_01//IO_PORTA_00 */
/* static void led_io_init(void) */
/* { */
/* gpio_set_die(LED_GPIO_PIN, 1); */
/* gpio_set_pull_down(LED_GPIO_PIN, 0); */
/* gpio_set_pull_up(LED_GPIO_PIN, 0); */
/* gpio_direction_output(LED_GPIO_PIN, 0); */
/* } */

// cdc send test
static void usb_cdc_send_test()
{
#if TCFG_USB_SLAVE_CDC_ENABLE
    log_info("-send test cdc data-");
    u8 cdc_test_buf[3] = {0x11, 0x22, 0x33};
    cdc_write_data(USB0, cdc_test_buf, 3);
    /* char test_char[] = "cdc test"; */
    /* cdc_write_data(USB0, test_char, sizeof(test_char)-1); */
#endif
}

extern void mem_stats(void);
static void spple_timer_handle_test(void)
{
    log_info("not_bt");
    //	mem_stats();//see memory
    //sys_timer_dump_time();
}

static const ble_init_cfg_t trans_data_ble_config = {
#if DOUBLE_BT_SAME_MAC
    .same_address = 1,
#else
    .same_address = 0,
#endif
    .appearance = 0,
};


void auto_power_off(void)
{
#if (TCFG_HID_AUTO_SHUTDOWN_TIME)
    y_printf("%s",__func__);
    //无操作定时软关机
    if(app_var.g_auto_shutdown_timer == 0){
        app_var.g_auto_shutdown_timer = sys_timeout_add(NULL, spple_set_soft_poweroff, TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
#endif
}


void modify_poweroff(void)
{
    log_info("%s[%d]",__func__,app_var.g_auto_shutdown_timer);
    if(app_var.g_auto_shutdown_timer){
        sys_timer_modify(app_var.g_auto_shutdown_timer,TCFG_HID_AUTO_SHUTDOWN_TIME * 1000);
    }
}

void auto_power_off_delete(void)
{
    if(app_var.g_auto_shutdown_timer){
        sys_timer_del(app_var.g_auto_shutdown_timer);
        app_var.g_auto_shutdown_timer = 0;
    }
}


/*************************************************************************************************/
/*!
 *  \brief      app start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/\
void lcd_bcak_on(void)
{
    // LCD_PWR_H();
    LCD_BLC_L();
}

void lcd_device_init(void)
{
    LCD_PWR_L();
    lcd_spi_init();
    LCD_Init();
    LCD_DMA_Clear(BLACK);
    // DispColor(COLOR_BLACK);   
    sys_timeout_add(NULL,lcd_bcak_on,300);
}

static void spple_app_start()
{
    log_info("=======================================");
    log_info("-----------spp_and_le demo-------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

    if (enter_btstack_num == 0) {
        enter_btstack_num = 1;
        clk_set("sys", BT_NORMAL_HZ);

//有蓝牙
#if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
        u32 sys_clk =  clk_get("sys");
        bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

#if TCFG_USER_EDR_ENABLE
        btstack_edr_start_before_init(NULL, 0);
#if USER_SUPPORT_PROFILE_HCRP
        __change_hci_class_type(BD_CLASS_PRINTING);
#endif
#if DOUBLE_BT_SAME_MAC
        //手机自带搜索界面，默认搜索到EDR
        __change_hci_class_type(BD_CLASS_TRANSFER_HEALTH);//
#endif
#endif

#if TCFG_USER_BLE_ENABLE
        btstack_ble_start_before_init(&trans_data_ble_config, 0);
#endif

        btstack_init();

#else
//no bt,to for test
        sys_timer_add(NULL, spple_timer_handle_test, 1000);
#endif
    }
    /* 按键消息使能 */
    sys_key_event_enable();
#if TCFG_SOFTOFF_WAKEUP_KEY_DRIVER_ENABLE
    set_key_wakeup_send_flag(1);
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
    extern void usb_start();
    usb_start();
#endif

#if 1
    X_timer_scan(NULL);
    // LCD_PWR_ECG_H();
    collect_heart_sound_handle();
    lcd_device_init();
    // Display_row(0xf800);
    // DispBand();
    // Display_test1_demo();
    // DispFrame();
    // DispFrame_test();
    // wenzi_test();
    // charge_mode();
    // charg_full_mode();
    // collect_ok_mode();
    // collect_fail_mode();
    // collect_30s_mode();
    // syn_data_mode();
    // low_power_mode();
    // power_off_mode();
    // color_pic_test();
    // battery_pic_disp();
    disp_data_num();
    // syn_data_all_ok_mode();
    // flash_one_second_deal();
#endif

#if 1
    flash_one_second_handle();
    collect_timer_cnt_handle();
#endif


#if 1
    auto_power_off();
    full_vbat_detect_handle();
#endif

}
/*************************************************************************************************/
/*!
 *  \brief      app 状态机处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int spple_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_SPPLE_MAIN:
            spple_app_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY\n");
        break;
    }

    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      hci 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int spple_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

#if TCFG_USER_EDR_ENABLE
    bt_comm_edr_hci_event_handler(bt);
#endif

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_hci_event_handler(bt);
#endif
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      bt 连接状态处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int spple_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

#if TCFG_USER_EDR_ENABLE
    bt_comm_edr_status_event_handler(bt);
#endif

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_status_event_handler(bt);
#endif
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      按键事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void spple_key_event_handler(struct sys_event *event)
{
    /* u16 cpi = 0; */
    u8 event_type = 0;
    u8 key_value = 0;
    static u16 goto_poweroff_cnt;

    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);
        /*Change Case To Idle Demo*/
        
        if(app_var.power_off_flag == 1){
            return;
        }

        if (event_type == KEY_EVENT_DOUBLE_CLICK) {
            // DispFrame();
            // draw_line_test2();
            // erase_block();
            erase_chip();
        }

        if (event_type == KEY_EVENT_CLICK) {
            // draw_point_test();
            // clear_list_test();
            // clear_screen();
            // ring_test();
            // LCD_DMA_Clear(BLACK);
            if(!app_var.power_off_flag){
                app_var.power_off_flag = 1; 
                power_off_mode();
            }
        }

        if(event_type == KEY_EVENT_TRIPLE_CLICK){
            // DispBand();
            // draw_line_test2();
            // draw_line_test();
            // draw_line_test3();
            history_data_read_handle();
        }
        if(event_type == KEY_EVENT_LONG){
            // write_equal_4x4_test();
            check_size_flash();
        }

        if(event_type == KEY_EVENT_HOLD){
            goto_poweroff_cnt++;
            r_printf("goto_poweroff_cnt = %d",goto_poweroff_cnt);
            if(goto_poweroff_cnt == 20){
                // spple_set_soft_poweroff();
                erase_chip();
                // cpu_reset();
                start_reset_vibrate();

            }
        }

        if(event_type == KEY_EVENT_UP){
            goto_poweroff_cnt = 0;
        }

    }

    if(event->arg == (void *)DEVICE_EVENT_FROM_DRAW){
#if 1            
        //先清除要显示的
        clear_list_heart_block(axis_xy.x_last,axis_xy.x_cur,COLOR_BLACK);
        //显示对应的折线
        LCD_DrawLine(axis_xy.x_last,axis_xy.y_last,axis_xy.x_cur,axis_xy.y_cur,WHITE);
#endif
    }

    if(event->arg == DEVICE_EVENT_FROM_HISTORY){
#if 1
        //要发送的历史数据
        mcu_send_history_app(&cur_history_data,sizeof(HISTORY_DATA));
#endif
    }

}

/*************************************************************************************************/
/*!
 *  \brief      app 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int spple_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        spple_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
#if (TCFG_USER_EDR_ENABLE || TCFG_USER_BLE_ENABLE)
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            spple_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            spple_bt_hci_event_handler(&event->u.bt);
        }
#endif
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, spple_set_soft_poweroff);
        }

#if TCFG_CHARGE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
#endif
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_spple_ops = {
    .state_machine  = spple_state_machine,
    .event_handler 	= spple_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_spple) = {
    .name 	= "spp_le",
    .action	= ACTION_SPPLE_MAIN,
    .ops 	= &app_spple_ops,
    .state  = APP_STA_DESTROY,
};

/*************************************************************************************************/
/*!
 *  \brief      注册控制是否允许系统进入sleep状态
 *
 *  \param      [in]
 *
 *  \return     1--可以进入sleep  0--不允许
 *
 *  \note
 */
/*************************************************************************************************/
static u8 spple_state_idle_query(void)
{
    return !is_app_spple_active;
}

REGISTER_LP_TARGET(spple_state_lp_target) = {
    .name = "spple_state_deal",
    .is_idle = spple_state_idle_query,
};

#endif


