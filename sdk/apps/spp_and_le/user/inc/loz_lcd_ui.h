#ifndef __LOZ_LCD_UI_H__
#define __LOZ_LCD_UI_H__


//字体的大小
#define pic16_16 16/8*16
#define pic32_16 32/8*16
#define pic32_32 32/8*32
#define pic40_32 40/8*32
#define pic64_32 64/8*32
#define pic128_32 128/8*32
#define pic160_80 160/8*80
#define pic120_24 120/8*24
#define pic128_24 128/8*24
#define pic40_40 40/8*40
#define pic32_40 32/8*40
#define pic24_40 24/8*40
#define pic144_50 144/8*50
#define pic56_8 56/8*8
#define pic88_8 88/8*8
#define pic88_16 88/8*16
#define pic16_24 16/8*24
#define pic48_18 48/8*18


// 定义常量颜色值
#define COLOR_RED    0xF800
#define COLOR_GREEN  0x07E0
#define COLOR_BLUE   0x001F
#define COLOR_WHITE  0xFFFF
#define COLOR_BLACK  0x0000

//像素颜色
#define WHITE        0xFFFF//白色
#define BLACK        0x0000//黑色
#define BLUE         0x001F//蓝色
#define BRED         0XF81F //紫色
#define GBLUE	     0X0E1F //浅蓝
#define RED          0xF800//红色
#define MAGENTA      0xF81F//紫红色
#define GREEN        0x07E0//绿色
#define CYAN         0x7FFF//青色
#define YELLOW       0xFFE0//黄色
#define BROWN 	     0XBC40 //褐色
#define BRRED 	     0XFC07 //棕红色
#define GRAY  	     0X8430 //灰色

#define PIC_INFO        8
#define PIC_WIDTH       3         
#define PIC_HEIGHT      5      

void loz_lcd_ui_printf(void);
int wenzi_test(void);
void wirte_equal(uint16_t x, uint16_t y, uint16_t width, uint16_t height,u16 color);
void write_equal_4x4_test(void);
void charge_mode(void);
void charg_full_mode(void);
void collect_ok_mode(void);
void dots_test(uint16_t x_axis, uint16_t y_axis);
void collect_fail_mode(void);
void collect_30s_mode(void);
void collect_full_mode(void);
void syn_data_mode(void);
void syn_data_all_ok_mode(void);
void low_power_mode(void);
void power_off_mode(void);
void draw_color_pic(uint16_t x, uint16_t y, const uint16_t *pic, uint16_t hegiht, uint16_t width);
void gantan_test(uint16_t x_axis, uint16_t y_axis);

void color_pic_test(void);
void flash_one_second_deal(void);
void battery_pic_disp(void);
void clear_screen(void);
void disp_data_num(void);
void store_data_disp(uint16_t x_axis, uint16_t y_axis);


void flash_one_second_handle(void);
void flash_one_second_delete(void);


#if 1
void LCD_DMA_Clear(u16 color);

#endif

typedef struct __PIC_INTO
{
    u16 x_s;
    u16 x_e;
    u16 y_s;
    u16 y_e;
    u16 height;
    u16 width;

}PIC_INTO;




#endif





