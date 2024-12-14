

#include "includes.h"
#include "app_config.h"
#include "asm/cpu.h"
#include "asm/irq.h"
#include "asm/clock.h"
#include "system/timer.h"
#include "system/init.h"
#include "gpio.h"
#include "app_main.h"
#include "asm/br23.h"


/* #define LOG_TAG_CONST   TMR */
/* #define LOG_TAG         "[USER_TMR]" */
/* #define LOG_INFO_ENABLE */
/* #define LOG_DUMP_ENABLE */
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#include "debug.h"
/*
注意

timer 现在定义优先级为6 ，关总中断不关闭该优先级，
该中断里面使用函数 const 变量都必须定义在ram，否则会跑飞



 *
 */


#if (TCFG_UI_ENABLE && TCFG_LED7_RUN_RAM && (TCFG_UI_LED1888_ENABLE ||  TCFG_UI_LED7_ENABLE))
#endif
#define TCFG_USER_TIMER_ENABLE

#ifdef TCFG_USER_TIMER_ENABLE


struct timer_hdl {
    int index;
    int prd;
};

static struct timer_hdl hdl;

#define __this  (&hdl)

static const u32 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};

#define APP_TIMER_CLK           clk_get("timer")
#define MAX_TIME_CNT            0x7fff
#define MIN_TIME_CNT            0x100


#define TIMER_CON               JL_TIMER2->CON
#define TIMER_CNT               JL_TIMER2->CNT
#define TIMER_PRD               JL_TIMER2->PRD
#define TIMER_VETOR             IRQ_TIME2_IDX

#define TIMER_UNIT_MS           2 //1ms起一次中断
#define MAX_TIMER_PERIOD_MS     (1000/TIMER_UNIT_MS)
#define AT_VOLATILE_RAM_CODE    AT(.volatile_ram_code)

/*-----------------------------------------------------------*/


static void (*timer_led_scan)(void *param);


void app_timer_led_scan(void (*led_scan)(void *))
{
    timer_led_scan = led_scan;
}



static u32 resettime_cnt_50us = 0;
static u32 cnt_50us = 0;
static u32 cnt_1ms = 0;                     //追踪好这个变量，上升，平稳，下降，空闲
static u8 mode_flag = 0;
/////下面函数调用的使用函数都必须放在ram
___interrupt
AT_VOLATILE_RAM_CODE
static void timer5_isr()
{
    static u32 cnt;
    static u32 collect_cnt;


    TIMER_CON |= BIT(14);



}

int led7_timer_init()
{
    u32 prd_cnt;
    u8 index;

    printf("------------%s :%d", __func__, __LINE__);

    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = TIMER_UNIT_MS * (APP_TIMER_CLK / 1000) / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            break;
        }
    }
    // prd_cnt = 300;      //12us  定时器
    // prd_cnt = 600;      //25us  定时器
    // prd_cnt = 1200;      //50us  定时器
    // prd_cnt = 2400;      //100us  定时器
    prd_cnt = 4800;      //200us  定时器                    另外一种方式去做
    // prd_cnt = 9600;      //400us  定时器s
    // prd_cnt = 24000;      //1ms  定时器

    r_printf("the prd_cnt = %d",prd_cnt);
    __this->index   = index;
    __this->prd     = prd_cnt;

    TIMER_CNT = 0;
    TIMER_PRD = prd_cnt; //1ms
    request_irq(TIMER_VETOR, 7, timer5_isr, 0);
    irq_unmask_set(TIMER_VETOR, 0);// 把该中断设为不可屏蔽的

    TIMER_CON = (index << 4) | BIT(0) | BIT(3);

    printf("PRD : 0x%x / %d", TIMER_PRD, clk_get("timer"));

    return 0;
}
__initcall(led7_timer_init);


AT_VOLATILE_RAM_CODE
void delay_us_by_nop(u32 usec) 
{
    u32 sys = clk_get("sys");
    u32 cnt, div;
    if(usec == 1)     {  div = 30;}
    else if(usec == 2){  div = 12;}
    else if(usec == 3){  div = 8; }
    else if(usec < 10){  div = 6; }
    else              {  div = 5; }
    cnt = usec * (sys / 1000000L / div);
    while(cnt--){ asm volatile("nop"); }
}





#if 1   //定时器us级别延时

//ums timer
#define USER_UDELAY_TIMER JL_TIMER3//微秒延时使用的定时器 地址 可以其他的定时器
#define USER_UDELAY_TIMER_IRQ IRQ_TIME3_IDX//微秒延时使用的定时器 终端号

/**
 * @brief 初始化 自定义微秒延时定时器
 *
 */
void user_udelay_init(void){
    bit_clr_ie(USER_UDELAY_TIMER_IRQ);
}

/**
 * @brief 自定义微秒延时接口
 *
 * @param usec 延时时间 单位微秒
 */
__attribute__((weak)) void user_udelay(u32 usec)
{
    USER_UDELAY_TIMER->CON = BIT(14);//0x4000;
    USER_UDELAY_TIMER->CNT = 0;
    USER_UDELAY_TIMER->PRD = (24* usec/4)  ;//延时时间=时钟源频率/分频系数*计数次数
    USER_UDELAY_TIMER->CON = BIT(0)|BIT(3)|BIT(4);//BIT(0)定时计数模式 BIT(3):晶振为时钟源 BIT(4):4分频
    while ((USER_UDELAY_TIMER->CON & BIT(15)) == 0);
    USER_UDELAY_TIMER->CON = BIT(14);
}

void user_delay_us(u32 usec)
{
    user_udelay(usec);
}

#endif



#endif

