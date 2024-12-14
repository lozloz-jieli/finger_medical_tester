#ifndef __MATH_DEAL_H__
#define __MATH_DEAL_H__



//屏幕的宏定义处理
#define LIST_NUM                80
#define LCD_START               20
#define LCD_BASE_NUM            (LCD_START+(LIST_NUM-LCD_START)/2)             //屏幕开始描点中心坐标

//压缩电压参数幅值
#define VOLTAGE_MODE_DIV        4

//ADC的数据采集宏定义处理
#define BASE_UP_DATA            655                                            //ADC采集的初始化上限       640-740
#define BASE_NUM                655                                            //ADC采集的初始化中心基数       640-740
#define BASE_DOWN_DATA            655                                            //ADC采集的初始化下限       640-740

#define DECAY_NUM               2

#define DISPLAY_COLLECT_OK      3

typedef struct __AXIS_XY
{
    u16 x_last;     //打到上一个X点位
    u16 y_last;     //打到上一个Y点位
    u16 x_cur;     //当前X点位
    u16 y_cur;      //当前Y点位
}AXIS_XY;



void LCD_DrawLine( u16 x1, u16 y1, u16 x2, u16 y2,u16 line_colour );
void math_deal_printf(void);
void draw_point_test(void);
void draw_line_test(void);
void draw_line_test2(void);
void draw_line_test3(void);
void clear_list_test(void);
void clear_list_heart_block(uint16_t xs,uint16_t xe,uint16_t color);

void collect_timer_cnt_handle(void);
void collect_timer_cnt_delete(void);
void clear_axis_status(void);
u8 get_collect_ok_status(void);





#endif
