#include "includes.h"
#include "app_config.h"
#include "system/includes.h"
#include "system/timer.h"
#include "asm/spi.h"
#include "asm/mcpwm.h"
#include "lcd_drive.h"

#if TCFG_LCD_NV3022B_ENABLE

static void delay_ms(unsigned int ms);
void user_delay_us(u32 usec);

/* 初始化代码 */
static const InitCode LcdInit_code[] = {
    {0x01, 0},				// soft reset
    {REGFLAG_DELAY, 120},	// delay 120ms
    {0x11, 0},				// sleep out
    {REGFLAG_DELAY, 120},
    {0x36, 1, {0x00}},
    {0x3A, 1, {0x05}},
    {0xB2, 5, {0x0c, 0x0c, 0x00, 0x33, 0x33}},
    {0xB7, 1, {0x22}},
    {0xBB, 1, {0x36}},
    {0xC2, 1, {0x01}},
    {0xC3, 1, {0x19}},
    {0xC4, 1, {0x20}},
    {0xC6, 1, {0x0F}},
    {0xD0, 2, {0xA4, 0xA1}},
    /* {0xE0,14, {0x70,0x04,0x08,0x09,0x09,0x05,0x2A,0x33,0x41,0x07,0x13,0x13,0x29,0x2F}}, */
    /* {0xE1,14, {0x70,0x03,0x09,0x0A,0x09,0x06,0x2B,0x34,0x41,0x07,0x12,0x14,0x28,0x2E}}, */
    {0xE0, 14, {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}},
    {0xE1, 14, {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}},
    {0X21, 0},
    {0X2A, 4, {0x00, 0x00, 0x00, 0xEF}},
    {0X2B, 4, {0x00, 0x00, 0x00, 0xEF}},
    {0X29, 0},
    {REGFLAG_DELAY, 20},
    {0X2C, 0},
    {REGFLAG_DELAY, 20},
};


void TFT_Write_Cmd(u8 data)
{
    lcd_cs_l();
    lcd_rs_l();
    lcd_spi_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Data(u8 data)
{
    lcd_cs_l();
    lcd_rs_h();
    lcd_spi_send_byte(data);
    lcd_cs_h();
}

void TFT_Write_Map(char *map, u32 size)
{
    spi_dma_send_map(map, size);
}

void TFT_Set_Draw_Area(int xs, int xe, int ys, int ye)
{
    TFT_Write_Cmd(0x2A);
    TFT_Write_Data(xs >> 8);
    TFT_Write_Data(xs);
    TFT_Write_Data(xe >> 8);
    TFT_Write_Data(xe);

    TFT_Write_Cmd(0x2B);
    TFT_Write_Data(ys >> 8);
    TFT_Write_Data(ys);
    TFT_Write_Data(ye >> 8);
    TFT_Write_Data(ye);

    TFT_Write_Cmd(0x2C);

    lcd_cs_l();
    lcd_rs_h();
}

static void TFT_BackLightCtrl(u8 on)
{
#if (TCFG_BACKLIGHT_PWM_MODE == 1)
    //注意：duty不能大于prd，并且prd和duty是非标准非线性的，建议用示波器看着来调
    extern int pwm_led_output_clk(u8 gpio, u8 prd, u8 duty);
    if (on) {
        pwm_led_output_clk(TCFG_BACKLIGHT_PWM_IO, 10, 10);
    } else {
        pwm_led_output_clk(TCFG_BACKLIGHT_PWM_IO, 10, 0);
    }
#elif (TCFG_BACKLIGHT_PWM_MODE == 2)
    static u8 first_flag = 1;
    extern void mcpwm_set_duty(pwm_ch_num_type pwm_ch, pwm_timer_num_type timer_ch, u16 duty);
    if (first_flag) {
        first_flag = 0;
        lcd_mcpwm_init();
    }
    if (on) {
        mcpwm_set_duty(lcd_pwm_p_data.pwm_ch_num, lcd_pwm_p_data.pwm_timer_num, 0);//duty 占空比：0 ~ 10000 对应 100% ~ 0%
    } else {
        mcpwm_set_duty(lcd_pwm_p_data.pwm_ch_num, lcd_pwm_p_data.pwm_timer_num, 10000);//duty 占空比：0 ~ 10000 对应 100% ~ 0%
    }
#else
    if (on) {
        lcd_bl_h();
    } else {
        lcd_bl_l();
    }
#endif
}

static void TFT_EnterSleep()
{
    TFT_Write_Cmd(0x28);
    /* delay_ms(120); */
    TFT_Write_Cmd(0x10);
    delay_ms(120);
}

static void TFT_ExitSleep()
{
    TFT_Write_Cmd(0x11);
    delay_ms(120);
    TFT_Write_Cmd(0x29);
    /* delay_ms(120); */
}

static void delay_ms(unsigned int ms)
{
    delay_2ms((ms + 1) / 2);
}


static void lcd_reset()
{
    lcd_reset_h();
    delay_ms(1000);
    lcd_reset_l();
    delay_ms(1000);
    lcd_reset_h();
}
#define SPI_WriteComm(a)                       TFT_Write_Cmd(a)
#define SPI_WriteData(a)                       TFT_Write_Data(a)
void NV3022B_LCD_Init(void)
{

	//CS0=0;

	// SPI_RES=1;
	user_delay_us(100);

	// SPI_RES=0;
	user_delay_us(800);

	// SPI_RES=1;
	user_delay_us(800);



//************* Start Initial Sequence **********//

//display Setting
SPI_WriteComm(0xff);
SPI_WriteData(0xa5);
SPI_WriteComm(0x3a);
SPI_WriteData(0x05);
SPI_WriteComm(0x51);
SPI_WriteData(0x14);
SPI_WriteComm(0x53);
SPI_WriteData(0x11);
SPI_WriteComm(0x62);//HFP
SPI_WriteData(0x10);
SPI_WriteComm(0x86);
SPI_WriteData(0x00);//frc_en 01
SPI_WriteComm(0x87);
SPI_WriteData(0x1a);//4.7 gvdd
SPI_WriteComm(0x88);
SPI_WriteData(0x0e);//vcomh0
SPI_WriteComm(0x89);
SPI_WriteData(0x18);//vcoml
SPI_WriteComm(0x61); //HBP
SPI_WriteData(0x10);
SPI_WriteComm(0x93);
SPI_WriteData(0x12);
SPI_WriteComm(0x94);
SPI_WriteData(0x10);
SPI_WriteComm(0x95);
SPI_WriteData(0x10);
SPI_WriteComm(0x96);
SPI_WriteData(0x0e);
SPI_WriteComm(0xb2);
SPI_WriteData(0x0F);
SPI_WriteComm(0xb4);
SPI_WriteData(0x60);
SPI_WriteComm(0x91);
SPI_WriteData(0x10);
SPI_WriteComm(0xC1);
SPI_WriteData(0xF1);
SPI_WriteComm(0xC5);
SPI_WriteData(0xF8);
SPI_WriteComm(0xb5);
SPI_WriteData(0x00);
SPI_WriteComm(0xc3);
SPI_WriteData(0x11);//ss/gs
SPI_WriteComm(0x83);
SPI_WriteData(0x10);
//////////////////////gamma_set//////////////////////////////////////
SPI_WriteComm(0x2a);
SPI_WriteData(0x00);
SPI_WriteData(0x84);
SPI_WriteData(0x00);
SPI_WriteData(0x84);
SPI_WriteComm(0x2b);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x1f);
SPI_WriteComm(0x2c);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x11);//VPMIDP 15
SPI_WriteData(0x00);
SPI_WriteData(0x0b);//VRP1 10
SPI_WriteData(0x00);
SPI_WriteData(0x18);//VRP0 21
SPI_WriteData(0x00);
SPI_WriteData(0x05);//KP7 0
SPI_WriteData(0x00);
SPI_WriteData(0x0D);//KP6 1
SPI_WriteData(0x00);
SPI_WriteData(0x0E);//KP5 2
SPI_WriteData(0x00);
SPI_WriteData(0x0a);//KP4 6
SPI_WriteData(0x00);
SPI_WriteData(0x1e);//KP3 25
SPI_WriteData(0x00);
SPI_WriteData(0x18);//KP2 29
SPI_WriteData(0x00);
SPI_WriteData(0x1a);//KP1 30
SPI_WriteData(0x00);
SPI_WriteData(0x1e);//KP0 31
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteData(31-0x11);//VPMIDN 15
SPI_WriteData(0x00);
SPI_WriteData(31-0x19); //VRN0 21
SPI_WriteData(0x00);
SPI_WriteData(31-0x0c);//VRN1 10
SPI_WriteData(0x00);
SPI_WriteData(31-0x1e);//KN0 31
SPI_WriteData(0x00);
SPI_WriteData(31-0x19);//KN1 30
SPI_WriteData(0x00);
SPI_WriteData(31-0x16);//KN2 29
SPI_WriteData(0x00);
SPI_WriteData(31-0x1f);//KN3 25
SPI_WriteData(0x00);
SPI_WriteData(31-0x0a);//KN4 6
SPI_WriteData(0x00);
SPI_WriteData(31-0x0f);//KN5 2
SPI_WriteData(0X00);
SPI_WriteData(31-0x11);//KN6 1
SPI_WriteData(0x00);
SPI_WriteData(31-0x05);//KN7 0

SPI_WriteComm(0x2a);
SPI_WriteData(0);
SPI_WriteData(0);
SPI_WriteData(0);
SPI_WriteData(127);

SPI_WriteComm(0x2b);
SPI_WriteData(0);
SPI_WriteData(0);
SPI_WriteData(0);
SPI_WriteData(159);

SPI_WriteComm(0x83);
SPI_WriteData(0x00);
SPI_WriteComm(0xff);
SPI_WriteData(0x00);
user_delay_us(120);
SPI_WriteComm(0x11);
user_delay_us(120);
SPI_WriteComm(0x21);
SPI_WriteComm(0x36);
SPI_WriteData(0x00);
user_delay_us(120);
SPI_WriteComm(0x29);
user_delay_us(120);
/*  //9341v-2.4hsd ips
SPI_WriteComm(0xCF);
SPI_WriteData(0x00);
SPI_WriteData(0xC1);
SPI_WriteData(0X30);
SPI_WriteComm(0xED);
SPI_WriteData(0x64);
SPI_WriteData(0x03);
SPI_WriteData(0X12);
SPI_WriteData(0X81);
SPI_WriteComm(0xE8);
SPI_WriteData(0x85);
SPI_WriteData(0x00);
SPI_WriteData(0x79);
SPI_WriteComm(0xCB);
SPI_WriteData(0x39);
SPI_WriteData(0x2C);
SPI_WriteData(0x00);
SPI_WriteData(0x34);
SPI_WriteData(0x02);
SPI_WriteComm(0xF7);
SPI_WriteData(0x20);
SPI_WriteComm(0xEA);
SPI_WriteData(0x00);
SPI_WriteData(0x00);
SPI_WriteComm(0xC0);       //Power control
SPI_WriteData(0x12);     //VRH[5:0]
SPI_WriteComm(0xC1);       //Power control
SPI_WriteData(0x13);     //SAP[2:0];BT[3:0]
SPI_WriteComm(0xC5);       //VCM control
SPI_WriteData(0x22);
SPI_WriteData(0x35);
SPI_WriteComm(0xC7);       //VCM control2
SPI_WriteData(0xAF);
SPI_WriteComm(0x3A); // Memory Access Control
SPI_WriteData(0x55);
SPI_WriteComm(0x36); // Memory Access Control
SPI_WriteData(0x08);
SPI_WriteComm(0xB1);
SPI_WriteData(0x00);
SPI_WriteData(0x15);
SPI_WriteComm(0xB6); // Display Function Control
SPI_WriteData(0x0A);
SPI_WriteData(0xA2);
SPI_WriteComm(0xF2); // 3Gamma Function Disable
SPI_WriteData(0x02);//00
SPI_WriteComm(0x26); //Gamma curve selected
SPI_WriteData(0x01);
SPI_WriteComm(0xE0);       //Set Gamma
SPI_WriteData(0x0F);
SPI_WriteData(0x35);
SPI_WriteData(0x31);
SPI_WriteData(0x0B);
SPI_WriteData(0x0E);
SPI_WriteData(0x06);
SPI_WriteData(0x49);
SPI_WriteData(0xA7);
SPI_WriteData(0x33);
SPI_WriteData(0x07);
SPI_WriteData(0x0F);
SPI_WriteData(0x03);
SPI_WriteData(0x0C);
SPI_WriteData(0x0A);
SPI_WriteData(0x00);
SPI_WriteComm(0XE1);       //Set Gamma
SPI_WriteData(0x00);
SPI_WriteData(0x0A);
SPI_WriteData(0x0F);
SPI_WriteData(0x04);
SPI_WriteData(0x11);
SPI_WriteData(0x08);
SPI_WriteData(0x36);
SPI_WriteData(0x58);
SPI_WriteData(0x4D);
SPI_WriteData(0x07);
SPI_WriteData(0x10);
SPI_WriteData(0x0C);
SPI_WriteData(0x32);
SPI_WriteData(0x34);
SPI_WriteData(0x0F);
SPI_WriteComm(0x21);
SPI_WriteComm(0x11); //Exit Sleep
Delay(60);
SPI_WriteComm(0x29);     //Display on
*/
}

#define LCD_WIDTH 240
#define LCD_HIGHT 240
#define LINE_BUFF_SIZE  (10*2*LCD_WIDTH*2)
static u8 line_buffer[LINE_BUFF_SIZE] __attribute__((aligned(4)));

REGISTER_LCD_DRIVE() = {
    .name = "st7789v",
    .lcd_width = LCD_WIDTH,
    .lcd_height = LCD_HIGHT,
    .color_format = LCD_COLOR_RGB565,
    .interface = LCD_SPI,
    .column_addr_align = 1,
    .row_addr_align = 1,
    .dispbuf = line_buffer,
    .bufsize = LINE_BUFF_SIZE,
    .initcode = NULL,//LcdInit_code,
    .initcode_cnt =0,// sizeof(LcdInit_code) / sizeof(LcdInit_code[0]),
    .WriteComm = TFT_Write_Cmd,
    .WriteData = TFT_Write_Data,
    .WriteMap = TFT_Write_Map,
    .SetDrawArea = TFT_Set_Draw_Area,
    .Init = NV3022B_LCD_Init,
    .Reset = lcd_reset,
    .BackLightCtrl = TFT_BackLightCtrl,
    .EnterSleep = TFT_EnterSleep,
    .ExitSleep = TFT_ExitSleep,
};

#endif


