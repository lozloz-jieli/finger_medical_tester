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
#include "lcd.h"
#include "loz_lcd_ui.h"
#include "math_deal.h"
#include "lost.h"


extern void modify_poweroff(void);


#if 1//LE_DEBUG_PRINT_EN
#define log_info(x, ...)  printf("[MATH_DEAL]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

extern HISTORY_DATA history_data;

//画在160*80d的点上
void DrawPoint_Lcd(uint16_t x, uint16_t y, uint16_t color)
{
    // putchar('A');
    // DrawPoint(y+26,x+1,color);

#if 1
    if(y>55){
        y = 55;
    }
    else if(y<1){
        y = 1;
    }
#endif

    DrawPoint(80-y+26,x+1,color);
    // r_printf("x = %d,y = %d",x,y);
}


void Clear_Point_Lcd(uint16_t x, uint16_t y, uint16_t color)
{
    // putchar('A');
    // DrawPoint(y+26,x+1,color);
    DrawPoint(y+26,x+1,color);
    // y_printf("x = %d,y = %d",x,y);
}

void clear_list_heart(uint16_t x,uint16_t color)
{
    uint16_t i; 
    for(i = 25; i < COL; i++){
        Clear_Point_Lcd(x,i,color);
    }
}


void clear_list_heart_block(uint16_t xs,uint16_t xe,uint16_t color)
{
    int i;
    for(i = xs;i <= xe; i++){
        clear_list_heart(i,color);
    }
}


void clear_list_test(void)
{
    static flag;
    clear_list_heart(flag,COLOR_BLACK);
    flag++;

}



void draw_point_test(void)
{
    DrawPoint_Lcd(2,2,COLOR_GREEN);
    DrawPoint_Lcd(2,3,COLOR_GREEN);
    DrawPoint_Lcd(2,4,COLOR_GREEN);
    DrawPoint_Lcd(2,5,COLOR_GREEN);
    DrawPoint_Lcd(2,6,COLOR_GREEN);
}


/*
**********************************************************************
函数功能：画直线
函数形参：
函数返回值：None
备注：
日期：2024年10月14日
作者：lozloz
版本：V0.0
**********************************************************************
*/
// 使用Bresenham算法画直线
void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int dx1 = abs(dx), dy1 = abs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    int xe, ye;
  
    r_printf("-------------%d------------",__LINE__);
    if (dy1 <= dx1) {
        r_printf("-------------%d------------",__LINE__);

        if (dx >= 0) {
            x0 = x0; y0 = y0; x1 = x1;
        } else {
            x0 = x1; y0 = y1; x1 = x0;
        }
        r_printf("-------------%d------------",__LINE__);

        DrawPoint_Lcd(x0, y0, color);
        for (int i = 0; x0 < x1; i++) {
            x0++;
            if (px < 0) {
                px = px + 2 * dy1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    y0++;
                } else {
                    y0--;
                }
                px = px + 2 * (dy1 - dx1);
            }
            DrawPoint_Lcd(x0, y0, color);
        }
    } else {
        if (dy >= 0) {
            x0 = x0; y0 = y0; y1 = y1;
        } else {
            x0 = x1; y0 = y1; y1 = y0;
        }
        DrawPoint_Lcd(x0, y0, color);
        for (int i = 0; y0 < y1; i++) {
            y0++;
            if (py <= 0) {
                py = py + 2 * dx1;
            } else {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                    x0++;
                } else {
                    x0--;
                }
                py = py + 2 * (dx1 - dy1);
            }
            DrawPoint_Lcd(x0, y0, color);
        }
    }
}



/*
**********************************************************************
函数功能：画折线
函数形参：
函数返回值：None
备注：
日期：2024年10月14日
作者：lozloz
版本：V0.0
**********************************************************************
*/
// 描点画折线：连接多个点
void DrawPolyline(uint16_t points[][2], uint16_t pointCount, uint16_t color)
{
    y_printf("pointCount = %d",pointCount);
    for (uint16_t i = 0; i < pointCount - 1; i++) {
        uint16_t x0 = points[i][0];
        uint16_t y0 = points[i][1];
        uint16_t x1 = points[i + 1][0];
        uint16_t y1 = points[i + 1][1];
        LCD_DrawLine(x0, y0, x1, y1, color); // 逐点连接形成折线
    }
}

// int points[160][2] = {
//     // 时间(ms), 电压(最大值为60) (样例数据)
//     {0, 12}, {1, 15}, {2, 20}, {3, 25}, {4, 30},
//     {5, 28}, {6, 24}, {7, 22}, {8, 18}, {9, 15},
//     {10, 12}, {11, 14}, {12, 18}, {13, 22}, {14, 26},
//     {15, 30}, {16, 32}, {17, 28}, {18, 24}, {19, 20},
//     {20, 16}, {21, 14}, {22, 12}, {23, 10}, {24, 8},
//     {25, 6}, {26, 10}, {27, 16}, {28, 22}, {29, 28},
//     // ...
//     {30, 12}, {31, 15}, {32, 20}, {33, 25}, {34, 30},
//     {35, 28}, {36, 24}, {37, 22}, {38, 18}, {39, 15},
//     {40, 12}, {41, 14}, {42, 18}, {43, 22}, {44, 26},
//     {45, 30}, {46, 32}, {47, 28}, {48, 24}, {49, 20},
//     {50, 16}, {51, 14}, {52, 12}, {53, 10}, {54, 8},
//     {55, 6}, {56, 10}, {57, 16}, {58, 22}, {59, 28},    

//     {60, 12}, {61, 15}, {62, 20}, {63, 25}, {64, 30},
//     {65, 28}, {66, 24}, {67, 22}, {68, 18}, {69, 15},
//     {70, 12}, {71, 14}, {72, 18}, {73, 22}, {74, 26},
//     {75, 30}, {76, 32}, {77, 28}, {78, 24}, {79, 20},
//     {80, 16}, {81, 14}, {82, 12}, {83, 10}, {84, 8},
//     {85, 6}, {86, 10}, {87, 16}, {88, 22}, {89, 28},    
//     // 总共160个点，电压值最大值为60
// };


void draw_line_test(void)
{
#if 1    
    // 定义多个点的坐标，形成折线
    uint16_t points[][2] = {{1, 10}, {4, 30}, {7, 20}, {10, 50}, {13, 10},{16, 10}, {19, 30}, {22, 20}, {25, 50}, {28, 10},{31, 10}, {34, 30}, {37, 20}, {40, 50}, {43, 10},{46, 10},{49, 30},
                            {52, 10}, {55, 30}, {58, 20}, {61, 50}, {64, 10},{67, 10}, {70, 30}, {73, 20}, {76, 50}, {79, 10},{82, 10}, {85, 30}, {88, 20}, {91, 50}, {94, 10},{97, 10},{100, 30},
                            };
    
    // 用蓝色（0x001F）画出折线
    DrawPolyline(points, sizeof(points)/sizeof(points[0]), GBLUE);
#else
    uint16_t x0 = 10;
    uint16_t y0 = 10;
    uint16_t x1 = 150;
    uint16_t y1 = 70;
    DrawLine(x0, y0, x1, y1, COLOR_GREEN); // 逐点连接形成折线
    LCD_DrawLine(x0, y0, x1, y1, COLOR_GREEN);
#endif
}

void draw_line_test2(void)
{
#if 1    
    // 定义多个点的坐标，形成折线
    uint16_t points[][2] = {{1, 10}, {5, 30}, {9, 20}, {13, 50}, {17, 10},{21, 10}, {25, 30}, {29, 20}, {33, 50}, {37, 10},{41, 10}, {45, 30}, {49, 20}, {53, 50}, {57, 10},{61, 10},{65, 30},
                            {69, 10}, {73, 30}, {77, 20}, {81, 50}, {85, 10},{89, 10}, {93, 30}, {97, 20}, {101, 50}, {105, 10},{109, 10}, {113, 30}, {117, 20}, {121, 50}, {125, 10},{129, 10},{133, 30},
                            };
    
    // 用蓝色（0x001F）画出折线
    DrawPolyline(points, sizeof(points)/sizeof(points[0]), GBLUE);
#else
    uint16_t x0 = 10;
    uint16_t y0 = 10;
    uint16_t x1 = 150;
    uint16_t y1 = 70;
    DrawLine(x0, y0, x1, y1, COLOR_GREEN); // 逐点连接形成折线
    LCD_DrawLine(x0, y0, x1, y1, COLOR_GREEN);
#endif
}

void draw_line_test3(void)
{
#if 1    
    // 定义多个点的坐标，形成折线
    uint16_t points[][2] = {{1, 10}, {3, 30}, {5, 20}, {7, 50}, {9, 10},{11, 10}, {13, 30}, {15, 20}, {17, 50}, {19, 10},{21, 10}, {23, 30}, {25, 20}, {27, 50}, {29, 10},{31, 10},{33, 30},
                            {35, 10}, {37, 30}, {39, 20}, {41, 50}, {43, 10},{45, 10}, {47, 30}, {49, 20}, {51, 50}, {53, 10},{55, 10}, {57, 30}, {58, 20}, {59, 50}, {61, 10},{63, 10},{65, 30},
                            };
    
    // 用蓝色（0x001F）画出折线
    DrawPolyline(points, sizeof(points)/sizeof(points[0]), GBLUE);
#else
    uint16_t x0 = 10;
    uint16_t y0 = 10;
    uint16_t x1 = 150;
    uint16_t y1 = 70;
    DrawLine(x0, y0, x1, y1, COLOR_GREEN); // 逐点连接形成折线
    LCD_DrawLine(x0, y0, x1, y1, COLOR_GREEN);
#endif
}

void draw_line_test4(void)
{
#if 1    
    // 定义多个点的坐标，形成折线
    uint16_t points[][2] = {{1, 10}, {3, 30}, {5, 20}, {7, 50}, {9, 10},{11, 10}, {13, 30}, {15, 20}, {17, 50}, {19, 10},{21, 10}, {23, 30}, {25, 20}, {27, 50}, {29, 10},{31, 10},{33, 30},
                            {35, 10}, {37, 30}, {39, 20}, {41, 50}, {43, 10},{45, 10}, {47, 30}, {49, 20}, {51, 50}, {53, 10},{55, 10}, {57, 30}, {58, 20}, {59, 50}, {61, 10},{63, 10},{65, 30},
                            };
    
    // 用蓝色（0x001F）画出折线
    DrawPolyline(points, sizeof(points)/sizeof(points[0]), GBLUE);
#else
    uint16_t x0 = 10;
    uint16_t y0 = 10;
    uint16_t x1 = 150;
    uint16_t y1 = 70;
    DrawLine(x0, y0, x1, y1, COLOR_GREEN); // 逐点连接形成折线
    LCD_DrawLine(x0, y0, x1, y1, COLOR_GREEN);
#endif
}

/*******************************
*函数名称：void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
*函数功能：画线函数
*参数说明：x1,y1:起点坐标
*          x2,y2:终点坐标
*          colour:线的颜色
*
*******************************/
void LCD_DrawLine( u16 x1, u16 y1, u16 x2, u16 y2,u16 line_colour )
{
    u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    // y_printf("uRow = %d,uCol = %d",uRow,uCol);

    if(delta_x>0){
        incx=1; //设置单步方向
    }
    else if(delta_x==0){
        incx=0;//垂直线
    }
    else {
        incx=-1;
        delta_x=-delta_x;
    }

    if(delta_y>0){
        incy=1;
    }
    else if(delta_y==0){
        incy=0;//水平线
    }
    else{
        incy=-1;
        delta_y=-delta_y;
    }


    if( delta_x>delta_y){
        distance=delta_x; //选取基本增量坐标轴
    }
    else{
        distance=delta_y;
    } 

    for(t=0;t<=distance+1;t++ )//画线输出
    {
        // lcd_draw_point( uRow,uCol,line_colour );//画点
        DrawPoint_Lcd(uRow,uCol,line_colour);
        xerr+=delta_x ;
        yerr+=delta_y ;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}

u8 get_collect_ok_status(void)
{
    if(elec_heart.heart_second == 30 && app_var.disp_collect_ok < DISPLAY_COLLECT_OK){
         return 1;
    }
    else{
        return 0;
    }
}

/*
**********************************************************************
函数功能：传输历史数据完成后重新测量数据
函数形参：
函数返回值：None
备注：
日期：2024年10月11日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void trans_history_over_flag(void)
{
    r_printf("%s",__func__);
    elec_heart.heart_second = 0;            //心跳的计时清零
    heart_var.x_flag = 0;                   //画点X轴清零
    app_var.disp_collect_ok = 0;           //30s显示采集成功清零
}

/*
**********************************************************************
函数功能：手指触摸计数开始
函数形参：
函数返回值：None
备注：
日期：2024年10月11日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 collect_timer_id;

void file_vm_deal(void)
{
    history_data.file++;                    //记录一次文本数据
    write_history_file();
    g_printf("history_data.file= %d loz_exflash_var.temp_file = %d",history_data.file,loz_exflash_var.temp_file);
}

void collect_timer_cnt_deal(void)
{
    // heart_var.drop_flag = heart_drop_out_read();


    if(loz_exflash_var.history_flag == 1){
        return;                             //正在上传历史数据，等上传完后者等蓝牙断卡或者停止才能继续下一步；
    }


#if 1     //开机等待
    if(elec_heart.power_second > 4){

    }
    else if(elec_heart.power_second == 4){
        elec_heart.power_second++;
        return;
    }
    else{
        elec_heart.power_second++;
        return;
    }
#endif

#if 1     //处理30s条件满足事件
    if(heart_var.drop_flag == 0){                         //手指脱落
        // putchar('T');
        // printf("heart_var.drop_flag = %d,app_var.heart_status = %d",heart_var.drop_flag,app_var.heart_status);
        if(app_var.heart_status){                         //已经开始画点了
            if(elec_heart.heart_second < 30 && elec_heart.heart_second >0){
                clear_screen();
                collect_fail_mode();
                clear_axis_status();
                buzzer_ring_status_handle(BUZZ_RING_ONCE);
                clear_30s_buffer();                        //不写入flash！！！
                
            }
            else{
                app_var.disp_collect_all_ok = 1;
                clear_screen();
                collect_ok_mode();
                clear_axis_status();
                buzzer_ring_status_handle(BUZZ_RING_ONCE);
                clear_30s_buffer();                     //记录一次文本数据
            }
            app_var.disp_collect_fail = 1;          //采集失败标志位
            app_var.heart_status = 0;              //心跳 画点的状态
            elec_heart.heart_second = 0;            //心跳的计时清零
            heart_var.x_flag = 0;                   //画点X轴清零
            app_var.disp_collect_ok = 0;           //30s显示采集成功清零
        }        
        return;
    }else{
        modify_powerof();
        app_var.disp_collect_all_ok = 0;
        // putchar('t');
        if(!app_var.heart_status){
            clear_screen();
            flash_one_second_deal();
            app_var.heart_status = 1;
            app_var.disp_collect_fail = 0;
            buzzer_ring_status_handle(BUZZ_RING_ONCE);            
        }
#if 1    ///到了30s显示采集成功
        if(elec_heart.heart_second == 30){
            buzzer_ring_status_handle(BUZZ_RING_TWICE);
            if(app_var.disp_collect_ok < DISPLAY_COLLECT_OK){
                if(!app_var.disp_collect_ok){
                    clear_screen();
                    file_vm_deal();
                }
                collect_30s_mode();
                clear_axis_status();
                heart_var.x_flag = 0;
                app_var.disp_collect_ok++;
                if(app_var.disp_collect_ok == DISPLAY_COLLECT_OK){
                    clear_screen();         //退出采集ok
                }
                return;                     //这里很容易有bug，后面看看怎么优化               
            }
        }        
#endif

    }
#endif

    elec_heart.heart_second++;                    //秒数累计

    if(elec_heart.heart_second == 30){
                                                //采集完成
    }

    if(elec_heart.heart_second > 300){
        elec_heart.heart_second = 300;
    }
}


void collect_timer_cnt_handle(void)
{
    if(collect_timer_id == 0){
        collect_timer_id = sys_timer_add(NULL,collect_timer_cnt_deal ,1000);
    }
}


void collect_timer_cnt_delete(void)
{
    if(collect_timer_id){
        sys_timer_del(collect_timer_id);
        collect_timer_id = 0;

    }
}


/********************************************************************************************************************************************** */
/*
**********************************************************************
函数功能:处理心电手指测量IO口初始化
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
void ECG_drop_port_init(void)
{
#if 0    
    gpio_set_pull_down(TCFG_ECG_DROP_OUT_PIN, 0);
    gpio_set_pull_up(TCFG_ECG_DROP_OUT_PIN, 0);    
    gpio_set_direction(TCFG_ECG_DROP_OUT_PIN, 1);
    gpio_set_die(TCFG_ECG_DROP_OUT_PIN, 1);
#else
    adc_add_sample_ch(TCFG_ECG_DROP_OUT_CH);          //注意：初始化AD_KEY之前，先初始化ADC

    gpio_set_die(TCFG_ECG_DROP_OUT_PIN, 0);
    gpio_set_direction(TCFG_ECG_DROP_OUT_PIN, 1);
    gpio_set_pull_down(TCFG_ECG_DROP_OUT_PIN, 0);
    gpio_set_pull_up(TCFG_ECG_DROP_OUT_PIN, 0);
#endif    
}     


/*
**********************************************************************
函数功能:处理心电手指测量是否脱落
函数形参：
函数返回值：None
备注：
日期：2024年12月06日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u8 heart_drop_out_read(void)
{
    int drop_adc_value;
    drop_adc_value = adc_get_voltage(TCFG_ECG_DROP_OUT_CH);
    y_printf("drop_adc_value = %d",drop_adc_value);
    if(drop_adc_value < 1000 || drop_adc_value > 1800){
        return 0;
    }
    else{
        return 1;
    }
}

/*
**********************************************************************
函数功能：手指脱落检测处理
函数形参：
函数返回值：None
备注：
日期：2025年1月2日
作者：lozloz
版本：V0.0
**********************************************************************
*/
u16 drop_out_timer_id;

void drop_out_filter_deal(void)
{
    int last_ret_value;
    int ret_value;    
    ret_value = heart_drop_out_read();

//消抖处理
    static u32 filter_cnt;
    if(ret_value == last_ret_value){
        // putchar('F');
        if(filter_cnt>3){
            heart_var.drop_flag =  ret_value;
        }
        filter_cnt++;
    }else{
        // putchar('N');
        filter_cnt = 0;
    }
    last_ret_value = ret_value;   
    // r_printf("heart_var.drop_flag = %d",heart_var.drop_flag);

}


void drop_out_filter_handle(void)
{
    ECG_drop_port_init();
    if(drop_out_timer_id == 0){
        drop_out_timer_id = sys_timer_add(NULL,drop_out_filter_deal,500);
    }
}






void math_deal_printf(void)
{
    log_info("%s",__func__);
}





