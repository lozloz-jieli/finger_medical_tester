// PA9  key
// PA1  PWM1
// PB5  BLED

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

#define LOG_TAG_CONST       LOST
#define LOG_TAG             "[LOST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[LOST]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#define FAST_ADV_INTERVAL_MIN          (800)   //500ms            蓝牙连接是由这两个宏决定的，广播间隔短，连接块
#define SLOW_ADV_INTERVAL_MIN          (2400)  //1.5s

u32 adv_interval_time = FAST_ADV_INTERVAL_MIN; // 0.625ms单位
static int adv_fast_timer = 0;
static int adv_slow_timer = 0;
static int auto_off_timer = 0;
#define FAST_ADV_TIMEOUT  90 * 1000  // 180 * 1000           //180s
#define SLOW_ADV_TIMEOUT  90 * 1000  // 60 * 60 * 1000       //1小时
#define AUTO_OFF_TIMEOUT  7* 24 *3600 * 1000  // 7 * 24 * 3600 * 1000 //7天

#define ALERT_FRE           300
static int immediate_alert_timer = 0;
static int alert_timer_reach = 0;
static int aler_click_over = 0;

static u8 alert_st = 0;
#define REASON_DISCONNECT   1 // 1为断开报警
#define REASON_IMMEDIATE    2 // 2为主动报警
static u8 alert_reason = 0;
extern void clr_wdt(void);
extern int get_battery_level(void);
void low_power_detect_deal(void);

u8 ble_get_battery_level(void)
{
    /* set_change_vbg_value_flag(); */
    static u16 vbat = 310;
    u16 temp = 0;
    u16 level = 0;
    temp = get_vbat_level();
    if((temp > 220)&&(temp < 340)){
        vbat = temp;
    }
    if(vbat > 300){
        level = 100;
    }else if(vbat > 290){
        level = 75;
    }else if(vbat > 270){
        level = 50;
    }else if(vbat > 260){
        level = 25;
    }else if(vbat > 220){
        level = 0;
    }
    // log_info("%s[%d %d %d]", __func__, vbat, temp, level);
    return level;
}

void set_adv_interval_time(u8 f_s)
{
    log_info("%s[%d]", __func__, f_s);
    if(f_s){
        adv_interval_time = FAST_ADV_INTERVAL_MIN;
    }else{
        adv_interval_time = SLOW_ADV_INTERVAL_MIN;
    }
}
u32 get_adv_interval_time(void)
{
    log_info("%s[%d]", __func__, adv_interval_time);
    return adv_interval_time;
}
static void ble_auto_off_timer(void *priv)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    app_set_soft_poweroff();
    auto_off_timer = 0;
}
static void ble_adv_slow_timer(void *priv)
{
    log_info("%s[%d]", __func__, AUTO_OFF_TIMEOUT);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    adv_slow_timer = 0;
    if(auto_off_timer == 0){
        auto_off_timer = sys_timeout_add(NULL, ble_auto_off_timer, AUTO_OFF_TIMEOUT);
    }
}
static void ble_adv_fast_timer(void *priv)
{
    log_info("%s", __func__);
    extern void ble_adv_interval_set(u8 mode);
    ble_adv_interval_set(1);
    adv_fast_timer = 0;
}
void ble_adv_interval_set(u8 mode)
{
    r_printf("%s[%d]", __func__, mode);
    static u8 mode_is = 0xff;
    struct ble_server_operation_t *ble_opt;
    ble_get_server_operation_table(&ble_opt);
    if(mode_is == mode){
        return;
    }
    if(bt_ble_is_connected()){
        return;
    }
    mode_is = mode;
    if(mode == 1){
        adv_interval_time = SLOW_ADV_INTERVAL_MIN;
        pwm_led_mode_set(PWM_LED_USER_SLOW_ADV);
        if(adv_slow_timer == 0){
            log_info("%s[%d]", __func__, SLOW_ADV_TIMEOUT);
            adv_slow_timer = sys_timeout_add(NULL, ble_adv_slow_timer, SLOW_ADV_TIMEOUT);
        }

    }else{
        adv_interval_time = FAST_ADV_INTERVAL_MIN;
        /* pwm_led_mode_set(PWM_LED1_ON);// */
//        if(adv_fast_timer == 0){
//            log_info("%s[%d]", __func__, FAST_ADV_TIMEOUT);
//            adv_fast_timer = sys_timeout_add(NULL, ble_adv_fast_timer, FAST_ADV_TIMEOUT);
//        }
    }
    ble_opt->adv_enable(NULL, 0);
    ble_opt->adv_enable(NULL, 1);
}
void ble_fast_adv_to_slow(void)
{
    log_info("%s[%d]", __func__, adv_interval_time);
    if(adv_interval_time == FAST_ADV_INTERVAL_MIN){
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        if(adv_fast_timer == 0){
            log_info("%s[%d]", __func__, FAST_ADV_TIMEOUT);
            adv_fast_timer = sys_timeout_add(NULL, ble_adv_fast_timer, FAST_ADV_TIMEOUT);
        }
    }
}

void adv_timer_del(void)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    if(adv_fast_timer){
        sys_timeout_del(adv_fast_timer);
        adv_fast_timer = 0;
    }
    if(adv_slow_timer){
        sys_timeout_del(adv_slow_timer);
        adv_slow_timer = 0;
    }
    if(auto_off_timer){
        sys_timeout_del(auto_off_timer);
        auto_off_timer = 0;
    }
    disconnect_alert_del();
}

#define BZR_PWM_TIMER    JL_TIMER2
#define BZR_PWM_FRE      8500               //2731
#define BZR_PWM_IO       TCFG_BUZZER_PIN///IO_PORTA_01
#define BZR_DUTY         20


#define BZR_OUTCHANNEL                   6
#define BZR_OUTPUTCHANNEL_SEL(x)         SFR(JL_IOMAP->CON1, 4, 3, x)

static u32 bzr_duty = BZR_DUTY;
static void buzzer_ctrl(u8 en, u32 duty)
{
    log_info("%s[%d  %d]", __func__, en, duty);
    static u8 init = 0;
    duty = duty * 100;
    if(en){
        if(!init){
            timer_pwm_init(BZR_PWM_TIMER, BZR_PWM_FRE, duty, BZR_PWM_IO, 2);
            init = 1;
            set_enter_lp(0);
        }
        set_timer_pwm_duty(BZR_PWM_TIMER, duty);
    }else{
        timer_pwm_delete(BZR_PWM_TIMER, BZR_PWM_IO, BZR_PWM_FRE, duty);
        init = 0;
        set_enter_lp(1);
    }

}

void buzzer_ring(void)
{
   printf("%s", __func__);
    if(app_var.buzz_mute == 1){
        return;
    }

#if 1
    // timer_pwm_init(BZR_PWM_TIMER, BZR_PWM_FRE, 1500, BZR_PWM_IO, 2);
    timer_pwm_init(BZR_PWM_TIMER, BZR_PWM_FRE, 5000, BZR_PWM_IO, CH1_T2_PWM_OUT);    
#else
    mcpwm_test();
#endif
}


void buzzer_close(void)
{
    printf("%s", __func__);

#if 1
    timer_pwm_delete(BZR_PWM_TIMER, BZR_PWM_IO, BZR_PWM_FRE, 2000);
#else
    mcpwm_close_FMQ();
	// buzzer_ctrl(0, 0);
    // set_enter_lp(1);
    // os_time_dly(10);
#endif    
}

void ring_test(void)
{
    static u8 ring_flag;

    // printf("%s",__func__);
    if(!ring_flag){
        ring_flag = 1;
        buzzer_ring();
    }
    else{
        ring_flag = 0;
        buzzer_close();
    }
}


void open_ring(void)
{
    log_info("%s",__func__);
    clr_wdt();
    buzzer_ring();
    os_time_dly(10);
    buzzer_close();

    clr_wdt();

}


u16 timer_down_over_id;
void timer_down_deal(void)
{
    log_info("%s",__func__);
    static u8 flag;
    static u8 cnt;
    if(!flag){
        buzzer_ctrl(1, bzr_duty);
        flag = 1;
    }else{
        buzzer_ctrl(0, 0);
        flag = 0;
    }
    cnt++;
    if(cnt >= 6){
        cnt = 0;
        flag = 0;
         buzzer_ctrl(0, 0);
        sys_timer_del(timer_down_over_id);
        timer_down_over_id = 0;
    }
}

void timer_down_deal_handle(void)
{
    // clean_data();
    log_info("%s",__func__);
    if(timer_down_over_id == 0){
        timer_down_over_id = sys_timer_add(NULL,timer_down_deal,200);
    }
}

void buzzer_ring_over(void)
{
    log_info("%s",__func__);
    if(timer_down_over_id == 0){
        timer_down_over_id = sys_timer_add(NULL,timer_down_deal,200);
    }
}


void delete_buzzing_over(void)
{
    log_info("%s",__func__);
    if(timer_down_over_id){
        sys_timer_del(timer_down_over_id);
        timer_down_over_id = 0;
    }
    buzzer_ctrl(0, 0);
}

////APP  协议部分
static u8 linkloss = 60; ///延时报警值, 100ms单位
void set_link_loss(u8 value)
{
    log_info("%s[%d -> %d]", __func__, linkloss, value);
    if(value == 0){
        linkloss = 60;
    }
    linkloss = value;
}
u8 get_link_loss(void)
{
    log_info("%s[%d]", __func__, linkloss);
    return linkloss;
}

void immediateAlert_del(void)
{
    log_info("%s", __func__);
    alert_st = 0;
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    if(immediate_alert_timer){
        sys_timer_del(immediate_alert_timer);
        immediate_alert_timer = 0;
    }
}
static void immediateAlert_deal(void)
{
    log_info("%s[%d]", __func__, alert_st);
    if(alert_st){
        alert_st = 0;
        pwm_led_mode_set(PWM_LED_ALL_OFF);
        buzzer_ctrl(0, 0);
    }else{
        alert_st = 1;
        pwm_led_mode_set(PWM_LED1_ON);
        buzzer_ctrl(1, bzr_duty);
    }
}

void immediateAlert_close_buzzer_deal(void)
{
    buzzer_ctrl(0, 0);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    sys_timer_del(immediate_alert_timer);
    immediate_alert_timer = 0;
}

void immediateAlert_ctrl(u8 level)
{
    log_info("%s[%d]", __func__, level);
	static u8 alert_status = 0;
    if(level == 0){
        alert_reason = 0;
        alert_st = 0;
        buzzer_ctrl(0, 0);
        pwm_led_mode_set(PWM_LED_ALL_OFF);
		alert_status = 0;
        if(immediate_alert_timer){
            sys_timer_del(immediate_alert_timer);
            immediate_alert_timer = 0;
        }
    }else if((level == 1)||(level == 2)){
        alert_reason = REASON_IMMEDIATE;
        bzr_duty = BZR_DUTY;
        buzzer_ctrl(1, bzr_duty);
        alert_st = 1;
		alert_status = 1;
        if(immediate_alert_timer == 0){
            immediate_alert_timer = sys_timer_add(NULL,immediateAlert_deal,ALERT_FRE);
			alert_timer_reach = sys_timeout_add(NULL,immediateAlert_close_buzzer_deal,180*1000);
        }
    }else{
        alert_reason = REASON_IMMEDIATE;
        bzr_duty = BZR_DUTY/2;
        buzzer_ctrl(1, bzr_duty);
        alert_st = 1;
		alert_status = 1;
        if(immediate_alert_timer == 0){
            immediate_alert_timer = sys_timer_add(NULL,immediateAlert_deal,ALERT_FRE);
        }
    }
	aler_click_over = alert_status;
}

static u8 alert_en = 1;
void disconnect_alert_set(u8 en)
{
    log_info("%s[%d]", __func__, en);
    alert_en = en;
}

static u8 alert_para[3] = {0};
void set_alert_para(u8 *buf, u8 buf_size)
{
    log_info("%s", __func__);
    log_info_hexdump(buf, buf_size);
    memcpy(alert_para, buf, sizeof(alert_para));
}
u8 get_alert_para(u8 *buf, u8 buf_size)
{
    log_info("%s", __func__);
    log_info_hexdump(alert_para, sizeof(alert_para));
    if(buf){
        memcpy(buf, alert_para, buf_size);
    }
    return sizeof(alert_para);
}

static int disconn_alert_timer = 0;
static u32 alert_cnt = 0;
void disconnect_alert_del(void)
{
    log_info("%s", __func__);
    alert_st = 0;
    alert_reason = 0;
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    alert_cnt = 0;
    if(disconn_alert_timer){
        sys_timer_del(disconn_alert_timer);
        disconn_alert_timer = 0;
    }
}
static void disconnect_alert_deal(void)
{
    log_info("%s[%d %d]", __func__, 180000/ALERT_FRE, alert_cnt);
    if(alert_st){
        alert_st = 0;
        pwm_led_mode_set(PWM_LED_ALL_OFF);
        buzzer_ctrl(0, 0);
    }else{
        alert_st = 1;
        pwm_led_mode_set(PWM_LED1_ON);
        buzzer_ctrl(1, BZR_DUTY);
    }
    if(alert_cnt++ > 180000/ALERT_FRE){
        disconnect_alert_del();
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
    }
}
void ble_disconnect_deal(void)
{
    log_info("%s[%d]", __func__, alert_en);
    if(!alert_en) return;
    alert_reason = REASON_DISCONNECT;
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    alert_st = 1;
    alert_cnt = 0;
    if(disconn_alert_timer == 0){
        disconn_alert_timer = sys_timer_add(NULL, disconnect_alert_deal, ALERT_FRE);
    }
}

// UI功能
extern void clr_wdt(void);

void check_power_on_key_B(void)
{
    log_info("%s", __func__);
    u32 delay_10ms_cnt = 0;
    while (1) {
        clr_wdt();
        os_time_dly(1);
        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+[%d]", delay_10ms_cnt);
            if(delay_10ms_cnt == 110){
                pwm_led_mode_set(PWM_LED1_ON);
                buzzer_ctrl(1, BZR_DUTY);
            }else if(delay_10ms_cnt == 120){
                buzzer_ctrl(0, 0);
            }else if(delay_10ms_cnt == 140){
                buzzer_ctrl(1, BZR_DUTY);
            }else if(delay_10ms_cnt == 150){
                buzzer_ctrl(0, 0);
            }
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 150) {

                return;
            }
        } else {
            log_info("-");
            buzzer_ctrl(0, 0);
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
            return;
        }
    }
}

void check_power_on_key_A(void)
{
    log_info("%s", __func__);
    u32 delay_10ms_cnt = 0;
    gpio_direction_input(TCFG_IOKEY_POWER_ONE_PORT);
    gpio_set_pull_down(TCFG_IOKEY_POWER_ONE_PORT, 0);
    gpio_set_pull_up(TCFG_IOKEY_POWER_ONE_PORT, 1);
    gpio_set_die(TCFG_IOKEY_POWER_ONE_PORT, 1);
    while (1) {
        clr_wdt();
        os_time_dly(1);
        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+[%d]", delay_10ms_cnt);
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 100) { //150
                break;
            }
        } else {
            log_info("-");
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
            return;
        }
    }
    pwm_led_init_delay();
    clr_wdt();
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    os_time_dly(10);
    buzzer_ctrl(0, 0);
    os_time_dly(10);
    buzzer_ctrl(1, BZR_DUTY);
    os_time_dly(10);
    buzzer_ctrl(0, 0);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    clr_wdt();
}

static int goto_power_off = 0;
void lost_enter_soft_poweroff(void)
{
    log_info("%s", __func__);
    pwm_led_mode_set(PWM_LED_ALL_OFF);
    buzzer_ctrl(0, 0);
    pwm_led_close();
    sys_timeout_add(NULL, app_set_soft_poweroff, 1000);
}
void lost_enter_soft_poweroff_close_buzzer(void)
{
    log_info("%s", __func__);
    buzzer_ctrl(1, BZR_DUTY);
    pwm_led_mode_set(PWM_LED1_ON);
    sys_timeout_add(NULL, lost_enter_soft_poweroff, 1000);
}
static void check_power_off_key(u8 event_type, u8 key_value)
{
    log_info("%s[%d]", __func__, goto_power_off);
    if(goto_power_off == -1){
        return;
    }
    if((event_type == KEY_EVENT_HOLD)&&(key_value == 0)){
        goto_power_off++;
        P33_CON_SET(P3_PINR_CON, 0, 1, 0);//关闭长按复位
    }
    if((event_type == KEY_EVENT_UP)&&(key_value == 0)){
        if(goto_power_off != -1)
            goto_power_off = 0;
    }
    if(goto_power_off > 10){
        goto_power_off = -1;
        pwm_led_mode_set(PWM_LED_USER_POWER_OFF);
        sys_timeout_add(NULL, lost_enter_soft_poweroff_close_buzzer, 1200);
    }
}
static int bat_timer = 0;
static void bat_deal(void)
{
    log_info("%s", __func__);
    app_cmd_bat();
    bat_timer = 0;
}
static int key_click_timer = 0;
static u8 last_display = 0;
static void key_led_close(void)
{
    log_info("%s", __func__);
    /* pwm_led_mode_set(PWM_LED_USER_FAST_ADV); */
    /* pwm_led_mode_set(PWM_LED_ALL_OFF); */
    pwm_led_mode_set(last_display);
    buzzer_ctrl(0, 0);
    key_click_timer = 0;
}
static void key_click_deal(void)
{
    log_info("%s", __func__);

	#if 0
    if(alert_reason == REASON_IMMEDIATE){ //主动报警,按键不能终止
        return;
    }
	#endif

    if(disconn_alert_timer){
        disconnect_alert_del();
        pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        ble_adv_interval_set(0);
        return;
    }else if(aler_click_over){
		log_info("----------------------here are the click stop----------------------------");
		immediateAlert_ctrl(0);
        //pwm_led_mode_set(PWM_LED_USER_FAST_ADV);
        ble_adv_interval_set(0);
	}
    ble_adv_interval_set(0);
    last_display = pwm_led_display_mode_get();
    pwm_led_mode_set(PWM_LED1_ON);
    buzzer_ctrl(1, BZR_DUTY);
    app_cmd_alert();
	app_cmd_secret();
    if(key_click_timer == 0){
        key_click_timer = sys_timeout_add(NULL, key_led_close, 50);
    }
    if(bat_timer == 0){
        bat_timer = sys_timeout_add(NULL, bat_deal, 200);
    }
}
static void key_double_click_deal(void)
{
    log_info("%s", __func__);
    app_cmd_alert();
    app_cmd_alert();
}

void app_key_deal(u8 event_type, u8 key_value)
{
    log_info("%s[%d %d]", __func__, event_type, key_value);
    check_power_off_key(event_type, key_value);
    if(goto_power_off == -1){
        return;
    }
    ble_get_battery_level();
    if(event_type == KEY_EVENT_CLICK){
        log_info("KEY_EVENT_CLICK");
        key_click_deal();
    }
    if(event_type == KEY_EVENT_DOUBLE_CLICK){
        log_info("KEY_EVENT_DOUBLE_CLICK");
        /* key_double_click_deal(); */
    }
    if(event_type == KEY_EVENT_TRIPLE_CLICK){
        log_info("KEY_EVENT_TRIPLE_CLICK");
        immediateAlert_close_buzzer_deal();
    }
}





/*
**********************************************************************
函数功能：不同状态蜂鸣器响应
函数形参：None
函数返回值：None
备注：
日期：2024年04月19日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 timer_buzzer_ring_id;

void set_ring_cnt(u8 cnt)
{
    static buzzer_ring_flag;
    u8 ring_cnt = cnt-1;

    if(app_var.buzzer_cnt< ring_cnt){
        if(!buzzer_ring_flag){
            buzzer_close();
            buzzer_ring_flag = 1;
        }else{
            buzzer_ring();
            buzzer_ring_flag = 0;
            app_var.buzzer_cnt++;
        }
    }else{
        app_var.buzzer_cnt = 0;
        buzzer_close();
        buzzer_ring_status_delete();
    }    
}


void buzzer_ring_status_deal(void)
{
    r_printf("app_var.buzzer_status = %d",app_var.buzzer_status);

    if(app_var.buzzer_status == BUZZ_RING_TWICE){
        set_ring_cnt(2);
    }

    else if(app_var.buzzer_status == BUZZ_RING_THRID){
        set_ring_cnt(3);
    }

    else if(app_var.buzzer_status == BUZZ_RING_ONCE){
        buzzer_ring_status_delete();     //响一声
    }

}


void buzzer_ring_status_handle(u8 status)
{
    app_var.buzzer_status = status;
    buzzer_ring();

    if(timer_buzzer_ring_id == 0){
        timer_buzzer_ring_id = sys_timer_add(NULL,buzzer_ring_status_deal,CYCLE_BUZZE_TIMER);
    }
}



void buzzer_ring_status_delete(void)
{
    if(timer_buzzer_ring_id){
        buzzer_close();
        sys_timer_del(timer_buzzer_ring_id);
        timer_buzzer_ring_id = 0;
    }
}



/*
**********************************************************************
函数功能：闹钟时刻的蜂鸣器响应
函数形参：None
函数返回值：None
备注：
日期：2024年04月22日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 alarm_buzee_timer_id;
u16 alarm_buzee_timeout_id;

void alarm_buzze_deal(void)
{

    static u8 flag;
    if(!flag){
        buzzer_ring();
        flag = 1;
    }

    else{
        buzzer_close();
        flag = 0;
    }
    
}


void alarm_buzze_handle(void)
{
    y_printf("%s",__func__);
    if(alarm_buzee_timer_id == 0){
        alarm_buzee_timer_id = sys_timer_add(NULL,alarm_buzze_deal,300);
        alarm_buzze_time_arrive_close();
    }
    else{
        alarm_buzze_time_arrive_close();
    }

}

void alarm_buzze_delete(void)
{
    if(alarm_buzee_timer_id){
        buzzer_close();
        sys_timer_del(alarm_buzee_timer_id);
        alarm_buzee_timer_id = 0;
    }
}


/*
**********************************************************************
函数功能：闹钟持持续响应时间
函数形参：None
函数返回值：None
备注：
日期：2024年04月22日
作者：lozloz
版本：V0.0
**********************************************************************
*/

void alarm_buzze_time_arrive_close(void)
{
    if(alarm_buzee_timeout_id == 0){
        alarm_buzee_timeout_id = sys_timeout_add(NULL,alarm_buzze_delete,30*1000);
    }
    else{
        sys_timer_modify(alarm_buzee_timeout_id,30*1000);
    }
}


void alarm_buzze_time_delete(void)
{
    if(alarm_buzee_timeout_id){
        sys_timeout_del(alarm_buzee_timeout_id);
        alarm_buzee_timeout_id = 0;
    }

}


/*
**********************************************************************
函数功能：按键停止闹钟响应
函数形参：None
函数返回值：None
备注：
日期：2024年04月22日
作者：lozloz
版本：V0.0
**********************************************************************
*/

void alarm_buzze_click_close(void)
{
    alarm_buzze_delete();
    alarm_buzze_time_delete();
}





/*****************************************蜂鸣器应用层软件**********************************************/

/*
**********************************************************************
函数功能：蜂鸣器按照频率响应各个函数的处理了
函数形参：None
函数返回值：None
备注：
日期：2024年04月22日
作者：lozloz
版本：V0.0
**********************************************************************
*/
//定时器的id号
u16 buzz_heart_fre_timer_id;

//频率响应的定时器删除函数
void buzz_heart_fre_delete(void)
{
    if(buzz_heart_fre_timer_id){
        sys_timer_del(buzz_heart_fre_timer_id);
        buzz_heart_fre_timer_id = 0;
    }
}

//频率响应定时器的时间更改函数
void buzz_heart_fre_modify(void)
{
    if(buzz_heart_fre_timer_id){
        sys_timer_modify(buzz_heart_fre_timer_id,calc_heart_fre());
    }
}

//定时器时间的计算函数
u8 calc_heart_fre(void)
{
    u8 timer_cycle;
    timer_cycle = elec_heart.heart_data * 1000 / 60;

    r_printf("timer_cycle = %d",timer_cycle);
    
    return timer_cycle;
}

//定时器频率的事件处理函数
void buzz_heart_fre_deal(void)
{
    static u8 fliter_cnt;
    static u8 ring_flag;

    if(!ring_flag){

    }


    if(elec_heart.heart_data != elec_heart.last_heart_data){
        fliter_cnt++;                                                                   //过滤
        if(fliter_cnt == 4){                                                            //过滤次数为4
            buzz_heart_fre_modify();
            elec_heart.last_heart_data = elec_heart.heart_data;
        }
    }else{
        fliter_cnt = 0;
    }
}

//定时器频率的定时器注册函数
void buzz_heart_fre_handle(void)
{
    if(!(elec_heart.heart_data>20 && elec_heart.heart_data<250)){
        r_printf("error heart_data return");
        return;
    }


    if(buzz_heart_fre_timer_id == 0){
        buzz_heart_fre_timer_id = sys_timer_add(NULL,buzz_heart_fre_deal,calc_heart_fre());
    }
}


/*
**********************************************************************
函数功能：蜂鸣器复位响应
函数形参：None
函数返回值：None
备注：
日期：2024年04月22日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 countdown_start_buzzer_id;
#define TIME_VIBRATER_DURATION  3*1000


void stop_reset_vibrate(void)   //  开振动提示
{
    log_info("stop_reset_vibrate");
	buzzer_close();
	countdown_start_buzzer_id = 0;
    cpu_reset();
}

void start_reset_vibrate(void)
{
  // start vibrating  for 300ms
  // add
    log_info("start_reset_vibrate");
    buzzer_ring();
    if(countdown_start_buzzer_id == 0){
        countdown_start_buzzer_id = sys_timeout_add(NULL,stop_reset_vibrate,TIME_VIBRATER_DURATION);
    }

}























/*****************************************蜂鸣器应用层软件**********************************************/












